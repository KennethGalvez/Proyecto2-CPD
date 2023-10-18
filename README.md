# Proyecto2-CPD


# Cambio de librería #include <rpc/des_crypt.h>
La librería no es accesible desde sistema modernos UNIX `#include <rpc/des_crypt.h>`. De manera que debe reemplazarse por alguna otra. Por ejemplo, la biblioteca OpenSSL ofrece opciones para encriptación/desencriptación DES. 


## Instalación de OpenSSL en Ubuntu

```bash
sudo apt-get update
```

```bash
sudo apt-get install libssl-dev
```

\*Asegurarse que la red no bloqueé la descarga de paquetes. 

## Actualización de script en función de la librería

1. Cambiar el header `#include <rpc/des_crypt.h>` por: 
```C
#include <openssl/des.h>
```
2. Cambiar la lógica de las funciones `decrypt` y `encrypt`
```C
void decrypt(long key, char *ciph, int len){
    DES_key_schedule schedule;
    DES_cblock keyBlock;
    
    // Prepara la clave DES
    memcpy(&keyBlock, &key, sizeof(key));
    DES_set_key_unchecked(&keyBlock, &schedule);

    // Descifra
    DES_ecb_encrypt((const_DES_cblock *)ciph, (DES_cblock *)ciph, &schedule, DES_DECRYPT);
}

void encrypt(long key, char *ciph, int len){
    DES_key_schedule schedule;
    DES_cblock keyBlock;
    
    // Prepara la clave DES
    memcpy(&keyBlock, &key, sizeof(key));
    DES_set_key_unchecked(&keyBlock, &schedule);

    // Cifra
    DES_ecb_encrypt((const_DES_cblock *)ciph, (DES_cblock *)ciph, &schedule, DES_ENCRYPT);
}
``` 

# Compilar y ejecutar

## Kenny

```bash
mpicc -w -o bruteforce bruteforce.c -lssl -lcrypto
```

```bash
mpirun -n <numero_de_procesos> ./bruteforce
```


## lp

```bash
gcc bruteforce.c -I/usr/local/include -pthread -L/usr/local/lib -Wl,-rpath -Wl,/usr/local/lib -Wl,--enable-new-dtags -lmpi -lssl -lcrypto -o bruteforce
```