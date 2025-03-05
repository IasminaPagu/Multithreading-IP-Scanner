# Multithreading-IP-Scanner

Se va implementa un program ce va scana toate adresele IP dintr-un interval specificat de
utilizator și va afișa care IP-uri reprezintă sisteme active și de asemenea pe ce port sunt acestea
active.
- Utilizatorul va putea specifica următorii parametrii:
o Intervalul de adrese IP
o O listă de porturi sau un interval de porturi ce vor fi scanate

**Implementarea acestui mecanism consta in urmatoarele lucruri:**
- se vor scana toate adresele IP introduse de uitlizator
- pentru fiecare adresa IP se va crea un thread detached, pentru ca scanarea adreselor sa se faca in paralel, utilizand paradigma de "multithreading"
- pentru fiecare thread creat se va apela functia "verificare_adresa_ip_activa()", in aceasta functie se va creea un socket de tip client.
**Optimizare:**  daca adresa IP nu este activa, functia connect() poate avea un timeout mare implicit, iar pentru a evita acest lucru am setat un timeout de 5 secunde folosind functia setsockopt().


**Atentie:** nu se vor creea un numar infinit de thread-uri detached, ci se va crea un numar specificat in macro-ul MAX_THREADS 10, adica in sistem nu pot exista mai mult de 10 thread-uri care ruleaza simultan. Pentru a nu exista simultan mai mult de MAX_THREADS threads, am folosit un semafor, a carui valoare se incrementeaza atunci cand se creeaza un thread si se decrementeaza atunci cand un thread isi termina executia. Daca valoarea depaseste capacitatea maxima de MAX_THREADS => se blocheaza executia pana cand un thread isi termina executia.
- la final, pentru a putea incheia programul, este necesar ca toate threadurile sa isi fi terminat executia, task realizat cu ajutorul functiei "verificare_finish_all_detached_threads()".

  
**Compilare si rulare**
gcc -Wall -o p implementare.c

./p

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


