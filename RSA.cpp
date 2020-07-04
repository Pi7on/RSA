#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//valori massimi per evitare problemi di overflow
//in caso di implementazione realistica al numero viene assegnata una memoria dinamica in base alla grandezza
#define ACCURACY 5
#define SINGLE_MAX 10000
#define EXPONENT_MAX 1000

//PROTOTIPI (definite sotto al main)
int modpow(long long a, long long b, int c); //calcola a^b mod c --> applica la chiave di de/cifratura ad un messaggio

int jacobi(int a, int n); //calcola il simbolo di Jacobi (a,n) --> usato in probablePrime()
int solovayPrime(int a, int n); // Check whether a is a Euler witness for n --> usato in probablePrime()
int probablePrime(int n, int k); //Testa se n è probabilmente primo, usando un accuratezza di k

int randPrime(int n); //ritorna un numero primo random
int gcd(int a, int b); //massimo comun divisore

int randExponent(int phi, int n); //trova esponente random x tra 3 e n-1 tale che gcd(x, phi) = 1 (condizioni che esponente pubblico deve rispettare)
int inverse(int n, int modulus); //calcola n^-1 mod m (modulo inverso) usando il metodo di euclide esteso --> usato per ricavare esponente privato da quello pubblico

int encode(int m, int e, int n); //cifra messaggio m usando esponente pubblico e modulo pubblico, c = m^e mod n
int decode(int c, int d, int n); //decifra cittogramma c usando esponente privato e modulo pubblico, m = c^d mod n

int main(void){
	//int m = 66;
	int m = 42; //messaggio originale
	int cypher, plain; //var per messaggio cifrato e decifrato
	int p, q; //due numeri primi
	int n; //prodotto di p*q, modulo pubblico
	int phi;
	int e, d; //esponente pubblico e privato

	srand(time(NULL)); //seed pseudo-random usando l'orario
	
	printf("Messaggio originale: %d [ASCII: %c]\n\n", m, m);
	
	p = randPrime(SINGLE_MAX); //genera primo numero primo
	printf("Trovato primo fattore primo, p = %d", p);
	getchar();
		
	q = randPrime(SINGLE_MAX); //genera secondo numero primo
	printf("Trovato secondo fattore primo, q = %d\n", q);
	getchar();
		
	n = p * q; //prodotto
	printf("Trovato modulo, n = p*q = %d\n", n);
		
		
	phi = (p - 1) * (q - 1); //calcola phi
	printf("Trovato phi = %d\n", phi);
	getchar();
	
	e = randExponent(phi, EXPONENT_MAX); //genera esponente pubblico (per cifrare)
	printf("Calcolo esponente pubblico, e = %d\nLa chiave pubblica e' (%d, %d) ", e, e, n);
	getchar();
	
	printf("\nCifro messaggio con esponente pubblico e \n");
	cypher = encode(m, e, n); //cifra messaggio e salva in crittogramma "cypher"
	printf("Messaggio cifrato: %d", cypher);
	getchar();
	
	d = inverse(e, phi); //genera esponente privato (per decifrare)
	printf("\nCalcolo esponente privato, d = %d\nLa chiave privata e' (%d, %d)", d, d, n);
	getchar();
	
	printf("\nDecifro messaggio usando esponente privato d \n");
	plain = decode(cypher, d, n); //decifra crittogramma e salva in "d"
	printf("messaggio decifrato: %d", plain);
	getchar();
	
	//controllo finale
	if(plain == m){
		printf("\n\n%d = %d\n", plain, m);
		printf("demo RSA completata con SUCCESSO!\n");
	}
	else{
		printf("\n\n%d != %d\n", plain, m);
		printf("Qualcosa è andato storto ... \n demo FALLITA.\n");
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}
//_______________________________________________________________________________________
//FUNZIONI
//calcola a^b mod c
int modpow(long long a, long long b, int c) { //uso numeri a 64 per evitare overflow
	int res = 1;
	while(b > 0) {
		if(b & 1) {
			res = (res * a) % c;
		}
		b = b >> 1;
		a = (a * a) % c;
	}
	return res;
}

//calcola il simbolo di Jacobi (a,n)
int jacobi(int a, int n) {
	int twos, temp;
	int mult = 1;
	while(a > 1 && a != n) {
		a = a % n;
		if(a <= 1 || a == n) break;
		twos = 0;
		while(a % 2 == 0 && ++twos) a /= 2; //Factor out multiples of 2 */
		if(twos > 0 && twos % 2 == 1) mult *= (n % 8 == 1 || n % 8 == 7) * 2 - 1;
		if(a <= 1 || a == n) break;
		if(n % 4 != 1 && a % 4 != 1) mult *= -1; //Coefficient for flipping
		temp = a;
		a = n;
		n = temp;
	}
	if(a == 0) return 0;
	else if(a == 1) return mult;
	else return 0; //a == n => gcd(a, n) != 1
}

int solovayPrime(int a, int n) { //usata in seguito all'interno di probablePrime()
	int x = jacobi(a, n);
	if(x == -1) x = n - 1;
	return x != 0 && modpow(a, (n - 1)/2, n) == x;
}

//Testa se n è probabilmente primo, usando un accuratezza di k
int probablePrime(int n, int k) {
	if(n == 2) return 1;
	else if(n % 2 == 0 || n == 1) return 0;
	while(k-- > 0) {
		if(!solovayPrime(rand() % (n - 2) + 2, n)) return 0;
	}
	return 1;
}

//Find a random (probable) prime between 3 and n - 1
int randPrime(int n) {
	int prime = rand() % n;
	n += n % 2; /* n needs to be even so modulo wrapping preserves oddness */
	prime += 1 - prime % 2;
	while(1) {
		if(probablePrime(prime, ACCURACY)) return prime;
		prime = (prime + 2) % n;
	}
}

//massimo comun divisore (a, b)
int gcd(int a, int b) {
	int temp;
	while(b != 0) {
		temp = b;
		b = a % b;
		a = temp;
	}
	return a;
}

//Find a random exponent x between 3 and n - 1 such that gcd(x, phi) = 1
int randExponent(int phi, int n) {
	int e = rand() % n;
	while(1) {
		if(gcd(e, phi) == 1) return e;
		e = (e + 1) % n;
		if(e <= 2) e = 3;
	}
}

//Compute n^-1 mod m by extended euclidian method
int inverse(int n, int modulus) {
	int a = n, b = modulus;
	int x = 0, y = 1, x0 = 1, y0 = 0, q, temp;
	while(b != 0) {
		q = a / b;
		temp = a % b;
		a = b;
		b = temp;
		temp = x; x = x0 - q * x; x0 = temp;
		temp = y; y = y0 - q * y; y0 = temp;
	}
	if(x0 < 0) x0 += modulus;
	return x0;
}

//cifra messaggio m usando esponente pubblico e modulo pubblico, c = m^e mod n
int encode(int m, int e, int n) {
	return modpow(m, e, n);
}

//decifra cittogramma c usando esponente privato e modulo pubblico, m = c^d mod n
int decode(int c, int d, int n) {
	return modpow(c, d, n);
}
