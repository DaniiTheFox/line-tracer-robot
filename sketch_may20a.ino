#include <Servo.h>
#define US_TRIG1 22
#define US_TRIG2 24
#define US_ECHO1 25
#define US_ECHO2 26
// -----------------------------------------------------------------------
// -                       DEFINICION DE LAS VARIABLES GLOBALES          -
// -----------------------------------------------------------------------
Servo arm_serv;                   // Brazo con el servo para recojer las pelotas

int speedf = 85;                  // configuracion de la velocidad 
int n_min = 300;                  // configuracion del color negro
int v_max = 400;                  // configuracion del color verde
int rot_s = 3;
char last_mov = 's'; // last movement 
// S = static
// D = Derecha
// I = izquierda
// R = Recto

//---------------------------------------------------------
int motor_A1 = 2, motor_A2 = 3, motor_A3 = 4, motor_A4 = 5; // DEFINIR MOTORES A
int motor_B1 = 6, motor_B2 = 7, motor_B3 = 8, motor_B4 = 9; // DEFINIR MOTORES B

// --------------------------------------------------------
char sensor[3];         // definicion de los sensores
char sensor_old[3];     // este array guarda el estado antiguo
//
// [ DETECCION0 DETECCION1 DETECCION2]
//  --------------------------------
//   ^- El sensor esta dividido en 3 sensores miniatura
//      Estos recuperan un valor ANALOGICO
//      Estos valores analogicos permiten distinguir tonos de gris
//      Entonces la logica bajo su uso es si es menor que n_min <- entonces es negro
//      si no es menor que n_min pero es mayor que el maximo entonces es blanco
//      y el caso que queda tiene que ser si o si verde
//      Bajo esta logica finalmente solo se guarda la primera letra del color
// --------------------------------------------------------
/*
*****************************************
* DEFINICION DE PROTOTIPOS DE FUNCIONES *
*****************************************
*/
void turn_right();    // FUNCION PARA GIRAR A LA DERECHA
void turn_left();     // FUNCION PARA GIRAR A LA IZQUIERDA
void movef();         // FUNCION PARA AVANZAR
void mover();
void dief();          // DETENER COMPLETAMENTE EL CARRO
void detect_line();   // DETECCION DE LINEA Y GUARDAR EN ARRAY
void save_state();    // COPIAR EL ARRAY EN EL OTRO ESTADO
void servo_angle();   // MOVER EL BRAZO DEL SERVO MOTOR
void go_back();
void special_go_back();
void avoid();
float find_obst(int trig_pin,int echo_pin); // Echo pin
/*---------------------------------------------*/
void setup(){
 /* 
 *************************************************
 * DEFINIR LAS PARTES DE EL HARDWARE PARA SU USO *
 *************************************************
 */
 // -- [DEFINICION DE LOS PINES PARA MOTORES]
 pinMode(motor_A1, OUTPUT); pinMode(motor_A2, OUTPUT);
 pinMode(motor_A3, OUTPUT); pinMode(motor_A4, OUTPUT);
 
 pinMode(motor_B1, OUTPUT); pinMode(motor_B2, OUTPUT);
 pinMode(motor_B3, OUTPUT); pinMode(motor_B4, OUTPUT);
 Serial.begin(9600);
 // DEFINIR EL SERVOMOTOR EN EL PIN 10
 arm_serv.attach(10);
 // ---------------------------------------
 pinMode(US_TRIG1, OUTPUT); pinMode(US_ECHO1, INPUT);
 pinMode(US_TRIG2, OUTPUT); pinMode(US_ECHO2, INPUT);
}

void loop(){
 detect_line(); // OBTENER LA POSICION ACTUAL DE LA LINEA
 save_state();  // GUARDAR LA LINEA -- FUNCIONAL PARA DETECTAR LINEA PUNTEADA Y BLANCO ABSOLUTO
 /*
 *********************************************
 * SISTEMA QUE SE ENCARGA DE SEGUIR LA LINEA *
 *********************************************
 */
 /*
  Esta parte no esta completamente escrita porque hacen falta las pruebas
  pero la logica de esto es lo siguiente
  si detecta una linea al centro avanza, hasta que detecte que la linea no esta centrada
  en ese caso el robor tiene que dar la vuelta hacia la direccion donde sigue detectando la linea
  como existe la posibilidad de que se acabe la linea, lo que decidi hacer es guardar el estado anterior
  de esta forma puedo tomar una decision, si el estado anterior detectaba una linea recta entonces
  es una linea punteada lo que estabamos haciendo, en el caso contrario que no habia linea recta y era por ejemplo
  bbn significa que termino (puede haberse descarrilado ese caso aun se esta observando) decide morir
  para no alejarse mucho de la linea (temporal)
 */
 //delay(250);
 //dief();
 //delay(250);
if(find_obst(US_TRIG2, US_ECHO2)<15 && find_obst(US_TRIG2, US_ECHO2)>3 ){
  avoid();
}else{
  if((sensor[0] == 'b' && sensor[1] == 'n' && sensor[2] == 'b')           // SI ESTA AL CENTRO LA LINEA
  ){
   movef();
  }else if((sensor[0] == 'b' && sensor[1] == 'n' && sensor[2] == 'n')           // SI ESTA AL CENTRO LA LINEA
  ||(sensor[0] == 'b' && sensor[1] == 'b' && sensor[2] == 'n')
  ){ 
   turn_right(); 
  }else if((sensor[0] == 'n' && sensor[1] == 'n' && sensor[2] == 'b')           // SI ESTA AL CENTRO LA LINEA
  ||(sensor[0] == 'n' && sensor[1] == 'b' && sensor[2] == 'b')
  ){ 
   turn_left(); 
  }else{
   if((sensor_old[0] == 'b' && sensor_old[1] == 'n' && sensor_old[2] == 'n')           // SI ESTA AL CENTRO LA LINEA
   ||(sensor_old[0] == 'b' && sensor_old[1] == 'b' && sensor_old[2] == 'n')){
     turn_right(); 
   }else if((sensor_old[0] == 'n' && sensor_old[1] == 'n' && sensor_old[2] == 'b')           // SI ESTA AL CENTRO LA LINEA
   ||(sensor_old[0] == 'n' && sensor_old[1] == 'b' && sensor_old[2] == 'b')){
     turn_left();
   }else{
     if(last_mov != 's' && last_mov != 'r'){
       go_back();
     }else if(last_mov == 'r'){
       movef();
     }else{
       dief();
     }
   }
  }
 }
} 
/*
****************************************************************************************************
* FUNCIONES PARA MOVER LOS MOTORES -- UNICAMENTE SE BASAN EN LA DIRECCION A PARTIR DE LA VELOCIDAD *
****************************************************************************************************
*/
void movef(){
 analogWrite(motor_A1,0);analogWrite(motor_A2,speedf);
 
 analogWrite(motor_B2,0);analogWrite(motor_B1,speedf);

 analogWrite(motor_A3,0);analogWrite(motor_A4,speedf);
 
 analogWrite(motor_B4,0);analogWrite(motor_B3,speedf);
 last_mov = 'r';
}

void special_go_back(){
 analogWrite(motor_A1,speedf);analogWrite(motor_A2,0);
 
 analogWrite(motor_B2,speedf);analogWrite(motor_B1,0);

 analogWrite(motor_A3,speedf);analogWrite(motor_A4,0);
 
 analogWrite(motor_B4,speedf);analogWrite(motor_B3,0);
 //movef();
 do{
  Serial.println("------------ DOING THE RETURN ----------- ");
  Serial.println(sensor[0]);
  Serial.println(sensor[1]);
  Serial.println(sensor[2]);
  delay(10);
  detect_line(); // OBTENER LA POSICION ACTUAL DE LA LINEA
  //save_state();  // GUARDAR LA LINEA -- FUNCIONAL PARA DETECTAR LINEA PUNTEADA Y BLANCO ABSOLUTO
 }while((sensor[0] == 'b' && sensor[1] == 'b' && sensor[2] == 'b'));
}  // XD

void go_back(){
 analogWrite(motor_A1,speedf);analogWrite(motor_A2,0);
 
 analogWrite(motor_B2,speedf);analogWrite(motor_B1,0);

 analogWrite(motor_A3,speedf);analogWrite(motor_A4,0);
 
 analogWrite(motor_B4,speedf);analogWrite(motor_B3,0);
 do{
  //Serial.println("------------ DOING THE RETURN ----------- ");
  //Serial.println(sensor[0]);
  //Serial.println(sensor[1]);
  //Serial.println(sensor[2]);
  delay(100);
  detect_line(); // OBTENER LA POSICION ACTUAL DE LA LINEA
  save_state();  // GUARDAR LA LINEA -- FUNCIONAL PARA DETECTAR LINEA PUNTEADA Y BLANCO ABSOLUTO
 }while((sensor[0] == 'b' && sensor[1] == 'b' && sensor[2] == 'b'));
}  // XD


void mover(){
 analogWrite(motor_A1,speedf);analogWrite(motor_A2,0);
 
 analogWrite(motor_B2,speedf);analogWrite(motor_B1,0);

 analogWrite(motor_A3,speedf);analogWrite(motor_A4,0);
 
 analogWrite(motor_B4,speedf);analogWrite(motor_B3,0);
 last_mov = 'b';
}  // XD

void dief(){
 analogWrite(motor_A1,0);analogWrite(motor_A2,0);
 
 analogWrite(motor_B2,0);analogWrite(motor_B1,0);

 analogWrite(motor_A3,0);analogWrite(motor_A4,0);
 
 analogWrite(motor_B4,0);analogWrite(motor_B3,0);
 last_mov = 's';
}

void turn_right(){
 analogWrite(motor_A1,0);analogWrite(motor_A2,speedf*rot_s);
 
 analogWrite(motor_B2,speedf);analogWrite(motor_B1,0);

 analogWrite(motor_A3,speedf);analogWrite(motor_A4,0);
 
 analogWrite(motor_B4,0);analogWrite(motor_B3,speedf*rot_s);
 //while(1){Serial.println("right");delay(100);}
 delay(200);
 last_mov = 'd';
}

void turn_left(){                                       //                       ATRAS
 analogWrite(motor_A1,speedf);analogWrite(motor_A2, 0); // atras        |  [A1A2]--------[A3A4]
                                                        //              |           |
 analogWrite(motor_B2, 0);analogWrite(motor_B1,speedf*rot_s);// enfrente|           | 
                                                        //              |           |     - ORDEN DE LOS MOTORES
 analogWrite(motor_A3, 0);analogWrite(motor_A4,speedf*rot_s); // atras  |           |
                                                        //              |           |
 analogWrite(motor_B4,speedf);analogWrite(motor_B3,0);  // enfrente     |  [B3B4]--------[B1B2]
 //while(1){Serial.println("right");delay(100);}
 delay(200);
 last_mov = 'i';
}                                                       //                      FRENTE
/*
***********************************************************
* FUNCION DE LA DETECCION DE LA LINEA A PARTIR DEL SENSOR *
***********************************************************
*/
void detect_line(){
 int sens_val0 = analogRead(A0);    // ESTA PARTE SE ENCARGA DE PEDIRLE LOS VALORES AL ARDUINO
 int sens_val1 = analogRead(A1);    // LLAMA A LAS ENTRADAS ANALOGICAS 0 1 2
 int sens_val2 = analogRead(A2);    // ASI OBTENGO DE LOS 3 PEDAZOS DEL SENSOR LOS TONOS DE GRIS QUE DETECTAN
 /*
 -----------------------------------------------------------
 - MENSAJES DE DEBUG PARA COMPROBAR SI VE BIEN LOS COLORES -
 -----------------------------------------------------------
 */
 Serial.println("Sensors:"); // Esto es util al momento de la calibracion 
 Serial.println(sens_val0);  // Esto imprime los valores que estoy obteniendo en cada uno 
 Serial.println(sens_val1);  // de los censores
 Serial.println(sens_val2);
 // --------------------------
 /*
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 + ESTA PARTE SE ENCARGA DE LEER LOS VALORES PARA INTENTAR DEDUCIR EL COLOR +
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
 /*
 =================================
 = DETECCION DEL SENSOR NUMERO 0 =
 =================================
 */
 if(sens_val0 < n_min){                      // si es negro
   Serial.print("negro\n");
   sensor[0] = 'n';
 }
 if(sens_val0 > n_min && sens_val0 < v_max){ // si es verde
   Serial.print("verde\n");
   sensor[0] = 'v';
 }
 if(sens_val0 > v_max){                      // si es blanco
   Serial.print("blanco\n");
   sensor[0] = 'b';
 }
 /*
 =================================
 = DETECCION DEL SENSOR NUMERO 1 =
 =================================
 */
 if(sens_val1 < n_min){                      // si es negro
  Serial.print("negro\n");
   sensor[1] = 'n';
 }
 if(sens_val1 > n_min && sens_val1 < v_max){ // si es verde
  Serial.print("verde\n");
   sensor[1] = 'v';               
 }
 if(sens_val1 > v_max){                      // si es blanco
  Serial.print("blanco\n");
  sensor[1] = 'b';
 }
 /*
 =================================
 = DETECCION DEL SENSOR NUMERO 2 =
 =================================
 */
 if(sens_val2 < n_min){                      // si es negro
  Serial.print("negro\n");
  sensor[2] = 'n';
 }
 if(sens_val2 > n_min && sens_val2 < v_max){ // si es verde
  Serial.print("verde\n");
  sensor[2] = 'v';
 }
 if(sens_val2 > v_max){                      // si es blanco
  Serial.print("blanco\n");
  sensor[2] = 'b';
 }
 // LOSE PUEDEN ESTAR EN UN LOOP PERO TOMARIA MAS TIEMPO REESCRIBIR ESTA PARTE
 // NO LO VALE SI YA FUNCIONA XD
}
/*
========================================
= GUARDAR EL ULTIMO ESTADO DE LA LINEA =
========================================
*/
void save_state(){
 for(int i = 0; i < 3; i++){  // PARA CADA UNO DE LOS ELEMENTOS
  sensor_old[i] = sensor[i];  // COPIARLO EN OTRO ARRAY PARA PODER GUARDAR "LA VERSION VIEJA"
 }
}
/*
===========================================
= MOVIMIENTO DEL BRAZO CON UN SERVO MOTOR =
===========================================
*/
void servo_angle(int ang){
    arm_serv.write(ang);         // ESCRIBIR EL ANGULO EN EL SERVO
    delay(30);                   // UN DELAY PARA DARLE TIEMPO AL SERVO QUE SE MUEVA
    // Serial.println("debug message:\n");
    // Serial.println(ang);
}

/*
---------------------------------
- DETECCION DE LOS ULTRASONICOS -
---------------------------------
*/
float find_obst(int trig_pin,int echo_pin){
  long snd_dur;
  float r_dist;
  /*
  *****************************************
  * LIMPIAR LA SALIDA DE LOS ULTRASONICOS *
  *****************************************
  */
  digitalWrite(trig_pin, LOW);
  delayMicroseconds(2);
  /*
  ******************************************
  * EMITIR UN SONIDO POR 10 MICROSEGUNDOS  *
  ******************************************
  */
  digitalWrite(trig_pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig_pin, LOW);
  /*
  ******************************************
  *  ESCUCHAR EL ECO DEL SONIDO NUEVAMENTE * 
  ******************************************
  */
  snd_dur = pulseIn(echo_pin, HIGH); 
  /*
  ***************************
  * CALCULAR LA DISTANCIA   *
  ***************************
  */
  r_dist = (snd_dur * 0.034)/2; // CALCULAR LA DISTANCIA
  // DEVOLVER EL VALOR DE LA DISTANCIA
  Serial.println("Ultrasonic:");
  Serial.println(r_dist);
  return r_dist;
}

void avoid(){
mover();
  delay(700);
  turn_left();
  delay(800);
  movef();
  delay(1000);
  turn_right();
  delay(900);
  movef();
  delay(700);
  turn_right();
  delay(800);
  movef();
  delay(700);
  turn_left();
  delay(900);
  movef();
  delay(100);
  special_go_back();
}
