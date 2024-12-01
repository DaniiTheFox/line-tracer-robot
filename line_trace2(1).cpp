#define US_TRIG1 23
#define US_TRIG2 24
#define US_ECHO1 25
#define US_ECHO2 26

// -----------------------------------------------------------------------
// -                       DEFINICION DE LAS VARIABLES GLOBALES          -
// -----------------------------------------------------------------------

int speedf = 85;                  // configuracion de la velocidad 
int n_min = 200;                  // configuracion del color negro
int v_max = 400;                  // configuracion del color verde
int rot_s = 3;
int l_dist = 600;
int tolerance = 100;
char last_mov = 's'; // last movement 
// S = static
// D = Derecha
// I = izquierda
// R = Recto

//---------------------------------------------------------
int motor_A1 = 2, motor_A2 = 3, motor_A3 = 4, motor_A4 = 5; // DEFINIR MOTORES A
int motor_B1 = 6, motor_B2 = 7, motor_B3 = 8, motor_B4 = 9; // DEFINIR MOTORES B

// --------------------------------------------------------
char sensor[3][3];         // definicion de los sensores
// --------------------------------------------------------
char tile1[3][3] = {{'b','n','b'},{'b','n','b'},{'b','n','b'}}; // LINEA RECTA
char tile2[3][3] = {{'b','n','b'},{'b','n','n'},{'b','b','n'}}; // LINEA A LA DERECHA
char tile3[3][3] = {{'b','n','b'},{'n','n','b'},{'n','b','b'}}; // LINEA A LA IZQUIERDA
char tile4[3][3] = {{'b','n','b'},{'b','b','b'},{'b','b','b'}}; // LINEA PUNTEADA
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
int  lev_think(char sres[3][3], char tile[3][3]);
void turn_right();            // FUNCION PARA GIRAR A LA DERECHA
void turn_left();             // FUNCION PARA GIRAR A LA IZQUIERDA
void movef();                 // FUNCION PARA AVANZAR
void move_back();
void dief();                  // DETENER COMPLETAMENTE EL CARRO
void detect_line(char c[3][3],int ln);// DETECCION DE LINEA Y GUARDAR EN ARRAY
void servo_angle();           // MOVER EL BRAZO DEL SERVO MOTOR
void go_back();
int  going_to_fail(int);
void dont();
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
 // ---------------------------------------
 pinMode(US_TRIG1, OUTPUT); pinMode(US_ECHO1, INPUT);
 pinMode(US_TRIG2, OUTPUT); pinMode(US_ECHO2, INPUT);
}

void loop(){
 // -- crear la deteccion del tile 
 if(find_obst(US_TRIG2, US_ECHO2)>27){
  for(int i = 0; i < 3; i++){
   detect_line(sensor, i);        // Teoricamente esto deberia regresar una "imagen" de 3x3
   movef();                       //    B N B   B N N   B N N |
   delay(l_dist);                 //    B N B o B N B o B N N |
  }                               //    B N B   B N B   B N B |
  move_back();                    //   ------------------------
  delay(l_dist*3);
  dief();
  // -- comparacion de distancia levenshtein 
  if(lev_think(sensor, tile1)<=1){
   movef();
  }
  if(lev_think(sensor, tile2)<=1){
   turn_left();
  }
  if(lev_think(sensor, tile3)<=1){
   turn_right();
  }
 }else if(find_obst(US_TRIG2, US_ECHO2)>5 && find_obst(US_TRIG2, US_ECHO2)<27){
  dief();
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

void move_back(){
 analogWrite(motor_A1,speedf);analogWrite(motor_A2,0);
 
 analogWrite(motor_B2,speedf);analogWrite(motor_B1,0);

 analogWrite(motor_A3,speedf);analogWrite(motor_A4,0);
 
 analogWrite(motor_B4,speedf);analogWrite(motor_B3,0);
 last_mov = 'h';
}

void go_back(){
 int now = 0;
 analogWrite(motor_A1,speedf);analogWrite(motor_A2,0);
 
 analogWrite(motor_B2,speedf);analogWrite(motor_B1,0);

 analogWrite(motor_A3,speedf);analogWrite(motor_A4,0);
 
 analogWrite(motor_B4,speedf);analogWrite(motor_B3,0);
 do{
  if(going_to_fail(now)){
    dont();
  }
  delay(100);
  detect_line(sensor,0); // OBTENER LA POSICION ACTUAL DE LA LINEA
  now++;
  }while(sensor[0][0] == 'b' && sensor[1][0] == 'b' && sensor[2][0] == 'b' && now > tolerance);
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
 delay(500);
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
 delay(500);
 last_mov = 'i';
}                                                       //                      FRENTE
/*
***********************************************************
* FUNCION DE LA DETECCION DE LA LINEA A PARTIR DEL SENSOR *
***********************************************************
*/
void detect_line(char c[3][3], int ln){
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
   c[0][ln] = 'n';
 }
 if(sens_val0 > n_min && sens_val0 < v_max){ // si es verde
   Serial.print("verde\n");
   c[0][ln] = 'v';
 }
 if(sens_val0 > v_max){                      // si es blanco
   Serial.print("blanco\n");
   c[0][ln] = 'b';
 }
 /*
 =================================
 = DETECCION DEL SENSOR NUMERO 1 =
 =================================
 */
 if(sens_val1 < n_min){                      // si es negro
   Serial.print("negro\n");
   c[1][ln] = 'n';
 }
 if(sens_val1 > n_min && sens_val1 < v_max){ // si es verde
   Serial.print("verde\n");
   c[1][ln] = 'v';               
 }
 if(sens_val1 > v_max){                      // si es blanco
  Serial.print("blanco\n");
  c[1][ln] = 'b';
 }
 /*
 =================================
 = DETECCION DEL SENSOR NUMERO 2 =
 =================================
 */
 if(sens_val2 < n_min){                      // si es negro
  Serial.print("negro\n");
  c[2][ln] = 'n';
 }
 if(sens_val2 > n_min && sens_val2 < v_max){ // si es verde
  Serial.print("verde\n");
  c[2][ln] = 'v';
 }
 if(sens_val2 > v_max){                      // si es blanco
  Serial.print("blanco\n");
  c[2][ln] = 'b';
 }
 // LOSE PUEDEN ESTAR EN UN LOOP PERO TOMARIA MAS TIEMPO REESCRIBIR ESTA PARTE
 // NO LO VALE SI YA FUNCIONA XD
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
  return r_dist;
}

int lev_think(char sres[3][3], char tile[3][3]){
  int errors = 0;
  
  String a1 = sres[0][0] + sres[0][1] + sres[0][2] +
              sres[1][0] + sres[1][1] + sres[1][2] +
              sres[2][0] + sres[2][1] + sres[2][2] + "";
 
 String a2 = tile[0][0] + tile[0][1] + tile[0][2] +
             tile[1][0] + tile[1][1] + tile[1][2] +
             tile[2][0] + tile[2][1] + tile[2][2] + "";
 
 for(int x=0; x < 9; x++){
   if(a1[x] == a2[x]){
    errors++; 
   }
 }
 return errors;
} 

int going_to_fail(int f){
  detect_line(sensor,0);
  if(
    (sensor[0][0] == 'b' && sensor[1][0] == 'b' && sensor[2][0] == 'b')||
    (sensor[0][0] == 'v' && sensor[1][0] == 'v' && sensor[2][0] == 'v')
  ){
    if(f > tolerance){
      return 1;
    }
  }
  return 0;
}
