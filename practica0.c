//Librerias
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>

//Constantes
#define tamNombreInterfaz 20
#define tamMAC 6
#define tamIP 4
#define tamNetmask 4
#define tamTrama 1514
#define tamEtherType 2
#define tamDatos 1000
#define OK 0
#define ERROR -1
#define TRUE 1

//Apagar interfaz de red y prenderla con banderas ----- TAREA

//Estructura datosRed redefinida como red
typedef struct datosRed
{
  //Variables que contiene la estructura red
  int indice;
  int MTU;
  int metrica;
  unsigned char MACorigen[tamMAC];
  unsigned char IPorigen[tamIP];
  unsigned char mascaraSubred[tamNetmask];
} red;

//Estructura datosTrama redefinida como trama
typedef struct datosTrama
{
  //Variables que contiene la estructura trama
  unsigned char MACdestino[tamMAC];
  unsigned char tramaEnviar[tamTrama];
  unsigned char tramaRecibir[tamTrama];
  unsigned char etherType[tamEtherType];
} trama;

//Prototipos de funciones
void obtenerIndice (int *ds, struct ifreq *interfaz, red * datos);
void obtenerMAC (int *ds, struct ifreq *interfaz, red * datos);
void obtenerIP (int *ds, struct ifreq *interfaz, red * datos);
void obtenerNetmask (int *ds, struct ifreq *interfaz, red * datos);
void obtenerMTU (int *ds, struct ifreq *interfaz, red * datos);
void obtenerMetrica (int *ds, struct ifreq *interfaz, red * datos);
void obtenerDatos (int *ds, red * datos);
void agredarDatos (trama * arreglos);
void estructuraTrama (red * datos, trama * arreglos);
void imprimeTrama (int bytes, unsigned char trama[]);
void enviaTrama (int *ds, red * datos, trama * arreglos);
void recibeTrama (int *ds, red * datos, trama * arreglos);

//Función main que regresa int y no recibe parámetros
int
main ()
{
  //Variables de la función int
  red datos;
  trama arreglos;
  int packet_socket, i;
  //Se abre el socket
  packet_socket = socket (AF_PACKET, SOCK_RAW, htons (ETH_P_ALL));
  //Se limpia la pantalla
  system ("clear");
  //Si no se pudo abrir el socket abortamos el programa
  if (packet_socket == ERROR)
    {
      perror ("Error al abrir el socket: ");
      puts ("");
      exit (ERROR);
    }
  printf ("Exito al abrir el socket\n");
  //Obtenemos los datos de la interfaz de red
  obtenerDatos (&packet_socket, &datos);
  //Estructuramos la trama que vamos a envíar
  estructuraTrama (&datos, &arreglos);
  //Envíamos la trama
  enviaTrama (&packet_socket, &datos, &arreglos);
  //Recibimos trama(s)
  recibeTrama (&packet_socket, &datos, &arreglos);
  //Se cierra el socket
  close (packet_socket);
  //El programa terminó bien
  return OK;
}

//Función obtenerIndice que regresa void y recibe como parámetros el descriptor de socket (int), 
//la estructura interfaz (ifreq) y la estructura datos (red); todos por referencia
void
obtenerIndice (int *ds, struct ifreq *interfaz, red * datos)
{
  //Obtenemos el índice de la interfaz de red, si no se pudo obtener abortamos el programa
  if (ioctl (*ds, SIOCGIFINDEX, interfaz) == ERROR)
    {
      perror ("Error al obtener el indice: ");
      puts ("");
      exit (ERROR);
    }
  //Asignamos el índice obtenido en la estructura interfaz a la estructura datos
  datos->indice = interfaz->ifr_ifindex;
  //Imprimimos el índice
  printf ("El indice es: %d\n", datos->indice);
  return;
}

//Función obtenerMAC que regresa void y recibe como parámetros el descriptor de socket (int), 
//la estructura interfaz (ifreq) y la estructura datos (red); todos por referencia
void
obtenerMAC (int *ds, struct ifreq *interfaz, red * datos)
{
  //Variables locales de la función obtenerMAC
  int i;
  //Obtenemos la MAC de la interfaz de red, si no se pudo obtener abortamos el programa
  if (ioctl (*ds, SIOCGIFHWADDR, interfaz) == ERROR)
    {
      perror ("Error al obtener la MAC: ");
      puts ("");
      exit (ERROR);
    }
  //Asignamos la MAC obtenida en la estructura interfaz a la estructura datos
  memcpy (datos->MACorigen, interfaz->ifr_hwaddr.sa_data + 0, 6);
  //Imprimimos la MAC en hexadecimal
  printf ("La MAC es: ");
  for (i = 0; i < tamMAC; i++)
    {
      printf ("%.2x:", datos->MACorigen[i]);
    }
  puts ("");
  return;
}

//Función obtenerIP que regresa void y recibe como parámetros el descriptor de socket (int), 
//la estructura interfaz (ifreq) y la estructura datos (red); todos por referencia
void
obtenerIP (int *ds, struct ifreq *interfaz, red * datos)
{
  //Variables locales de la función int
  int i;
  //Obtenemos la IP de la interfaz de red, si no se pudo obtener abortamos el programa
  if (ioctl (*ds, SIOCGIFADDR, interfaz) == -1)
    {
      perror ("\nError al obtener la direccion IP: ");
      puts ("");
      exit (ERROR);
    }
  //Asignamos la IP obtenida en la estructura interfaz a la estructura datos
  memcpy (datos->IPorigen, interfaz->ifr_addr.sa_data + 2, 4);
  //Imprimimos la IP en decimal
  printf ("La IP es: ");
  for (i = 0; i < tamIP; i++)
    printf ("%d.", datos->IPorigen[i]);
  puts ("");
  //printf ("\nFamilia: %.4x\n", interfaz->ifr_netmask.sa_family); <-----VIENE EN EL LIBRO?????
  return;
}

//Función obtenerNetmask que regresa void y recibe como parámetros el descriptor de socket (int), 
//la estructura interfaz (ifreq) y la estructura datos (red); todos por referencia
void
obtenerNetmask (int *ds, struct ifreq *interfaz, red * datos)
{
  //Variables locales de la función obtenerNetmask
  int i;
  //Obtenemos la máscara de subred, si no se pudo obtener abortamos el programa
  if (ioctl (*ds, SIOCGIFNETMASK, interfaz) == -1)
    {
      perror ("\nError al obtener la mascara de subred: ");
      puts ("");
      exit (ERROR);
    }
  //A
  memcpy (datos->mascaraSubred, interfaz->ifr_netmask.sa_data + 2, 4);
  printf ("La mascara de subred es: ");
  for (i = 0; i < 4; i++)
    printf ("%d.", datos->mascaraSubred[i]);
  puts ("");
  //printf ("\nFamilia: %.4x\n", interfaz->ifr_netmask.sa_family); <-----VIENE EN EL LIBRO
  return;
}

void
obtenerMTU (int *ds, struct ifreq *interfaz, red * datos)
{
  if (ioctl (*ds, SIOCGIFMTU, interfaz) == -1)
    {
      perror ("Error al obtener el MTU: ");
      puts ("");
      exit (ERROR);
    }
  datos->MTU = interfaz->ifr_mtu;
  printf ("El MTU es: %d\n", datos->MTU);
  return;
}

void
obtenerMetrica (int *ds, struct ifreq *interfaz, red * datos)
{
  if (ioctl (*ds, SIOCGIFMETRIC, interfaz) == -1)
    {
      perror ("Error al obtener la metrica: ");
      puts ("");
      exit (ERROR);
    }
  datos->metrica = interfaz->ifr_metric;
  printf ("La metrica es: %d\n", datos->metrica);
  return;
}

//Función obtenerDatos que regresa void y que recibe como parámetros el descriptor de socket (int) y la estructura datos (red) por referencia
void
obtenerDatos (int *ds, red * datos)
{
  //Variables locales de la función obtenerDatos
  char nombre_Interfaz[tamNombreInterfaz];
  struct ifreq interfaz;
  int i;
  printf ("Inserta el nombre\n");
  scanf ("%s", nombre_Interfaz);
  strcpy (interfaz.ifr_name, nombre_Interfaz);
  //Obtener Indice
  obtenerIndice (ds, &interfaz, datos);
  //Obtener MAC
  obtenerMAC (ds, &interfaz, datos);
  //Obtener IP
  obtenerIP (ds, &interfaz, datos);
  //Obtener mascara de subred
  obtenerNetmask (ds, &interfaz, datos);
  //Obtener MTU
  obtenerMTU (ds, &interfaz, datos);
  //Obtener metrica
  obtenerMetrica (ds, &interfaz, datos);
  return;
}

void
agregarMAC (trama * arreglos)
{
  int i;
  printf ("Ingrese la MAC destino [XX:XX:XX:XX:XX:XX:]: ");
  for (i = 0; i < tamMAC; i++)
    scanf ("%2x:", &arreglos->MACdestino[i]);
  return;
}

void
agregarEtherType (trama * arreglos)
{
  int i;
  printf ("Ingrese el etherType [XX.XX.]: ");
  for (i = 0; i < tamEtherType; i++)
    scanf ("%2x.", &arreglos->etherType[i]);
  return;
}

//Función agregar datos que recibe .........
void
agredarDatos (trama * arreglos)
{
  int tam;
  char datos[tamDatos];
  memset (datos, 0, tamDatos);
  printf ("Ingrese los datos: ");
  fgets (datos, tamDatos, stdin);
  fgets (datos, tamDatos, stdin);
  tam = strlen (datos);
  memcpy (arreglos->tramaEnviar + 14, datos, tam);
  return;
}

//Función estructuraTrama que regresa void y que recibe como parámetros la estructura datos (red) por referencia, y la estructura arreglos (trama) por referencia
void
estructuraTrama (red * datos, trama * arreglos)
{
  memset (arreglos->tramaEnviar, 0, tamTrama);
  agregarMAC (arreglos);
  memcpy (arreglos->tramaEnviar + 0, arreglos->MACdestino, 6);
  memcpy (arreglos->tramaEnviar + 6, datos->MACorigen, 6);
  agregarEtherType (arreglos);
  memcpy (arreglos->tramaEnviar + 12, arreglos->etherType, 2);
  agredarDatos (arreglos);
  return;
}

//Función imprimeTrama que regresa void y recibe como parámetros el número de bytes que se imprimirán (int) y la trama que se enviará o recibirá (unsigned char*)
void
imprimeTrama (int bytes, unsigned char trama[])
{
  //Variables locales de la función imprimeTrama
  int i;
  for (i = 0; i < bytes; i++)
    {
      if (i % 16 == 0)
	puts ("");
      printf ("%.2x ", trama[i]);
    }
  puts ("");
  return;
}

//Función enviaTrama que regresa void y que recibe como parámetros el descriptor de socket, la estructura datos (red) por referencia, y la estructura arreglos (trama) por referencia
void
enviaTrama (int *ds, red * datos, trama * arreglos)
{
  //Variables locales de la función enviaTrama
  int tam;
  struct sockaddr_ll interfaz;
  memset (&interfaz, 0x00, sizeof (interfaz));
  interfaz.sll_family = AF_PACKET;
  interfaz.sll_protocol = htons (ETH_P_ALL);
  interfaz.sll_ifindex = datos->indice;
  tam =
    sendto (*ds, arreglos->tramaEnviar, 60, 0, (struct sockaddr *) &interfaz,
	    sizeof (interfaz));
  if (tam == ERROR)
    {
      perror ("Error al enviar la trama: ");
      //puts ("");
      exit (ERROR);
    }
  perror ("Exito al enviar la trama: ");
  //puts ("");
  printf ("**********La trama que se envío es**********");
  imprimeTrama (tam, arreglos->tramaEnviar);
  return;
}

void
recibeTrama (int *ds, red * datos, trama * arreglos)
{
  double usegundos = 0;
  int tam, i;
  while (usegundos <= 5)
  {
  tam = recvfrom (*ds, arreglos->tramaRecibir, 1514, MSG_DONTWAIT, NULL, 0);
  if (tam == ERROR)
    {
      perror ("Error al recibir: ");
      printf("Elapsed time: %.4lf\n", usegundos);
      //puts ("");
      //exit (ERROR);
    }
  if ((!memcmp (arreglos->tramaRecibir + 6, arreglos->MACdestino, 6)) && (!memcmp (arreglos->tramaRecibir + 12, arreglos->tramaEnviar+12, 2))  && (!memcmp (arreglos->tramaRecibir + 0, datos->MACorigen, 6)))
  {
  perror ("Exito al recibir la trama: ");
  //puts ("");
  printf ("**********La trama que se recibió es**********");
  imprimeTrama (tam, arreglos->tramaRecibir);
  break;
  }
  usegundos+=0.001f;
  usleep(1000);
  }
  return;
}

