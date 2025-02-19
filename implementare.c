#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<semaphore.h>
/*
    Se va implementa un program ce va scana toate adresele IP dintr-un interval specificat de
utilizator si va afisa care IP-uri reprezinta sisteme active si de asemenea pe ce port sunt acestea
active si adresa MAC a acestora.


    Utilizatorul va putea specifica urmatorii parametrii:
o Intervalul de adrese IP
o O lista de porturi sau un interval de porturi ce vor fi scanate
o Numar de threaduri care sa realizeze intreaga operatiune
*/

/*
observatie, intr un octet exista 256 de numere
si daca facem presupunerea ca subnetmask este 255.255.255.0, adica /24
inseamna ca pentru ultimul octet, adica b4 si b44, nu vom avea voie sa accesam :

.0 -- network adress
.1 -- default gateway
.255 -- broadcast adress

*/
#define MAX_PORTS 100
#define MAX_THREADS 10
sem_t sem ;
pthread_t threads[100];
volatile int threaduri_parcurse = 0;


typedef struct thread_arg{
    int index;
    int nr_porturi;
    int lista_porturi[MAX_PORTS];
} thread_arg;

char ip_adresses[100000][100]; // acesta este vectorul meu de adrese ip, care este declarat ca si o matrice, deoarece la fiecare index, am o adresa ip de tip char[100]
int dim = 0;                   // dimensiunea vectorului de adrese ip
int nr_threads = 0;

void separe_bytes_adresa_ip(char adresa[100], int *b1, int *b2, int *b3, int *b4)
{
    *b1 = atoi(strtok(adresa, "."));
    if (b1 == NULL)
    {
        perror("");
        exit(-2);
    }
    *b2 = atoi(strtok(NULL, "."));
    *b3 = atoi(strtok(NULL, "."));
    *b4 = atoi(strtok(NULL, "."));
}
void generare_adrese_ip(int b1, int b11, int b2, int b22, int b3, int b33, int b4, int b44)
{
    if (b1 < b11)
    {
        while (b1 < b11)
        {
            for (; b2 <= 255; b2++)
            {
                for (; b3 <= 255; b3++)
                {
                    for (; b4 < 255; b4++)
                    {
                        sprintf(ip_adresses[dim], "%d.%d.%d.%d", b1, b2, b3, b4);
                        dim++;
                    }
                    b4 = 2;
                }
                b3 = 0;
            }
            b1++;
            b2 = 0;
            b3 = 0;
            b4 = 2;
        }
    }
    if (b1 == b11)
    {
        if (b2 < b22)
        {
            while (b2 < b22)
            {
                for (; b3 <= 255; b3++)
                {
                    for (; b4 < 255; b4++)
                    {
                        sprintf(ip_adresses[dim], "%d.%d.%d.%d", b1, b2, b3, b4);
                        dim++;
                    }
                    b4 = 2;
                }
                b2++;
                b3 = 0;
                b4 = 2;
            }
        }
        if (b2 == b22)
        {
            if (b3 < b33)
            {
                while (b3 < b33)
                {
                    for (; b4 < 255; b4++)
                    {
                        sprintf(ip_adresses[dim], "%d.%d.%d.%d", b1, b2, b3, b4);
                        dim++;
                    }
                    b4 = 2;
                    b3++;
                }
            }
            if (b3 == b33)
            {
                while (b4 <= b44)
                {
                    sprintf(ip_adresses[dim], "%d.%d.%d.%d", b1, b2, b3, b4);
                    dim++;
                    b4++;
                }
            }
        }
    }
}
void verificare_adresa_ip_activa(char adresa_ip[], int port)
{
    int sockfd;                                         // file descriptor pentru socket
    struct sockaddr_in server;                          // structura sockaddr pentru stocarea adresei si portului destinatie la care se va realiza conexiunea
    struct timeval timeout;                             // structura pentru configurarea timpului de asteptare
    
    timeout.tv_sec = 5;                                 // setam 5 secunde
    timeout.tv_usec = 0;                                // fara microsecunde

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) // crearea socket-ului TCP
    {
        perror("socket");
        exit(1);
    }

    // Setarea timeout-ului pentru operatiile de scriere (inclusiv connect)
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout)) < 0)
    {
        perror("setsockopt");
        close(sockfd);
        exit(1);
    }

    memset(&server, 0, sizeof(server));                 // initializarea cu 0 a structurii sockaddr
    server.sin_family = AF_INET;                        // setarea modului AF_INET obligatoriu pentru aceasta structura
    server.sin_addr.s_addr = inet_addr(adresa_ip);      // setarea adresei ip la care sa se realizeze conexiunea
                                                        //(in acest caz 127.0.01 - localhost)
    server.sin_port = htons(port);                      // setarea portului
    
    if ((connect(sockfd, (struct sockaddr *)&server, sizeof(server))) < 0) // realizarea conexiunii cu serverul
    {
        perror("connect");
        printf("adresa ip %s nu este activa pe portul %d\n", adresa_ip, port);
    }
    else
    {
        printf("adresa ip %s este activa pe portul %d\n", adresa_ip, port);
    }
    close(sockfd); // se inchide socket-ul
}

void *thread_function(void *arg)
{
    thread_arg t = *(thread_arg *)arg;
    int nr_porturi = t.nr_porturi;
    int i = t.index;
    printf("am intrat in functia thread-ului %d\n", i);
    for (int j = 0; j < nr_porturi; j++)
    {
        verificare_adresa_ip_activa(ip_adresses[i], t.lista_porturi[j]);
    }
    if (sem_post(&sem) < 0)
    {
        perror(NULL);
        exit(EXIT_FAILURE);
    }
    free(arg);
    return NULL;
}

void afisare_adrese_ip()
{
    for (int i = 0; i < dim; i++)
    {
        printf("%s\n", ip_adresses[i]);
    }
    printf("\n");
}
void implementare(int nr_port,int lista_port[100]){
    while(threaduri_parcurse < dim){
        if (sem_wait(&sem) < 0)
        {
            perror(NULL);
            exit(EXIT_FAILURE);
        }
        thread_arg *t;
        t = (thread_arg*)malloc(sizeof(thread_arg));
        if(t == NULL){
            perror("");
            exit(-15);
        }
        t->nr_porturi = nr_port;
        t->index = threaduri_parcurse;
        for (int j = 0; j < nr_port; j++)
        {
            t->lista_porturi[j] = lista_port[j];
        }
        pthread_attr_t attr;
        int s = pthread_attr_init(&attr);
        if (s != 0)
        {
            perror("");
            exit(-11);
        }
        int val_return_setdetached = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        if (val_return_setdetached != 0)
        {
            perror("");
            exit(-13);
        }
        pthread_create(&threads[threaduri_parcurse], &attr, thread_function, (void *)t);
        printf("thread-uri parcurse este %d\n",threaduri_parcurse);
        threaduri_parcurse++;
    }
    //daca sem == 0, inseamna ca nu mai exista locuri libere in 
    //ideea e asa
    //inainte sa fac pthread_create verific daca mai exista locuri disponibile
    //daca nu mai exista locuri astept

    //in functia thread_funcion dupa ce fac verificarea respectiva, incrementez semaforul
    //=> o sa se stie ca poate sa se creeze un alt thread

    // eu o sa folosc sem_wait in aceasta functie
    //si sem_post in thread_function

    // sem_wait() decrements(locks) the semaphore pointed to by sem.
    // If the semaphore's value is greater than zero, then the decrement proceeds, and the function returns, immediately.
    // If the semaphore currently has the value zero, then the call blocks until either it becomes possible to perform the decrement
    // (i.e., the semaphore value rises above zero), or a signal handler interrupts the call.

    // sem_post() increments(unlocks) the semaphore pointed to by sem.If the semaphore's value consequently be‐ comes greater than zero, 
    // then another process or thread blocked in a sem_wait(3) call will be woken up and
    // proceed to lock the semaphore.
}
int main(int argc, char *argv[])
{

    int b1, b2, b3, b4, b11, b22, b33, b44;
    char adresa_ip_inceput[100], adresa_ip_final[100];
    int optiune;
    int nr_port = 0;
    int start, end;
    int lista_port[MAX_PORTS];

    printf("introduceti adresa ip de inceput\n");
    scanf("%s", adresa_ip_inceput);

    printf("introduceti adresa ip de final\n");
    scanf("%s", adresa_ip_final);

    printf("daca doriti sa introduceti porturi rand pe rand, apasati tasta 1, altfel, daca doriti sa introduceti o lista de porturi, apasati tasta 2\n");
    scanf("%d", &optiune);

    switch (optiune)
    {
    case 1:
        printf("introduceti nr de porturi = ");
        scanf("%d", &nr_port);
        for (int i = 0; i < nr_port; i++)
        {
            printf("port[%d] este = ", i);
            scanf("%d", &lista_port[i]);
        }
        break;
    case 2:
        printf("introduceti capatul de start al intervalului de porturi, start = ");
        scanf("%d", &start);
        printf("introduceti capatul de sfarsit end = ");
        scanf("%d", &end);
        for (int i = start; i <= end; i++)
        {
            lista_port[nr_port] = i;
            nr_port++;
        }
        break;

    default:
        printf("nu ati introdus comanda corespunzatoare\n");
    }
    printf("porturile sunt \n\n\n");
    for (int i = 0; i < nr_port; i++)
    {
        printf("%d\n", lista_port[i]);
    }

    separe_bytes_adresa_ip(adresa_ip_inceput, &b1, &b2, &b3, &b4);
    separe_bytes_adresa_ip(adresa_ip_final, &b11, &b22, &b33, &b44);

    generare_adrese_ip(b1, b11, b2, b22, b3, b33, b4, b44);
    afisare_adrese_ip();

    int dim_threads = 0; // variabila in care contorizez cate thread-uri o sa se creeze in total
    if (dim >= MAX_THREADS)
    {
        // inseamna ca am nevoie sa se creeze mai multe thread-uri decat limita maxima indicata de MAX_THREADS
        //  ceea ce este imposibil, deci o sa se creeze MAX_THREADS thread-uri
        //  si o sa se verifice care dintre aceste thread-uri a terminat de verificat adresa ip pe lista de porturi
        // iar in acest caz dezvolt implementarea mea
        dim_threads = MAX_THREADS;
    }
    else
    {
        dim_threads = dim;
    }
    int val_initializare_sem = sem_init(&sem,0,dim_threads);//mi am initializat semaforul si i am zis cate thread-uri au voie maxim in parcare
    if(val_initializare_sem < 0 ){
        perror("");
        exit(-14);
    }
    implementare(nr_port,lista_port);
   
    sleep(200);
    sem_destroy(&sem);
    return 0;
}
//!!notite
/*

exista mai multe porturi cunoscute
    20 - FTP - File Tranport Protocol (Data Transfer)
    21 - FTP - File Tranport Protocol (Command Control)
    22 - SSH - Secure Shell
    23 - Telnet
    25 -SMTP (Simple Mail Tranport Protocol)
    53 - DNS (Domain Name System)
    80 - HTTP
    110 - POP3(Post Office Protocol) used by email clients to retrive email from a server
    119 - NNTP(Network News Transport Protocol)
    123 - NTP (Network Time Protocol)
    143 - IMAP(Internet Message Accesss Protocol)
    161 - SNMP(Simple Network Management Protocol)
    194 - IRC(Internet Relay Chat)
    443 - HTTPS
*/

//!!ex rulare cod
/*
introduceti adresa ip de inceput
142.251.208.110
introduceti adresa ip de final
142.251.208.119
daca doriti sa introduceti porturi rand pe rand, apasati tasta 1, altfel, daca doriti sa introduceti o lista de porturi, apasati tasta 2
1
introduceti nr de porturi = 
3


port[0] este = 80
port[1] este = 443
port[2] este = 20
*/
