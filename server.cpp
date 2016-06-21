#include <nacl/crypto_box.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "devurandom.h"
#include <cstdlib>
#include <unistd.h>

void generatekeys(const std::string &publickeyfile,
                  const std::string &privatekeyfile)
{
	std::string publickey, privatekey;
	try {
		publickey = crypto_box_keypair(&privatekey);
	} catch(const char* e) {
		std::cerr << "could not generate keypair: " << e << std::endl;
		std::exit(-1);
	}
	std::fstream fprivkey(privatekeyfile.c_str(), std::ios::out | std::ios::binary);
	std::fstream fpubkey(publickeyfile.c_str(), std::ios::out | std::ios::binary);
	fprivkey << privatekey;
	fpubkey << publickey;
}

void loadkeys(const std::string &publickeyfile, const::std::string &privatekeyfile,
	      std::string &publickey, std::string &privatekey)
{
	std::fstream fprivkey(publickeyfile.c_str(), std::ios::in | std::ios::binary);
	std::fstream fpubkey(privatekeyfile.c_str(), std::ios::in | std::ios::binary);
	
	if (!fpubkey.is_open()) {
		std::cerr << "could not open " << publickeyfile << std::endl;
		std::exit(-1);
	}
	
	if (!fprivkey.is_open()) {
		std::cerr << "could not open " << privatekeyfile << std::endl;
		std::exit(-1);
	}
	
	std::stringstream streambuffer;
	streambuffer << fpubkey.rdbuf();
	publickey = streambuffer.str();
	
	if (publickey.length() < 1) {
		std::cerr << "could not read private key from " << publickeyfile
		          << std::endl << "please check filecontent" << std::endl;
		std::exit(-1);
	}
	
	streambuffer.str("");
	streambuffer << fprivkey.rdbuf();
	privatekey = streambuffer.str();
	
	if (privatekey.length() < 1) {
		std::cerr << "could not read private key from " << privatekeyfile
		          << std::endl << "please check filecontent" << std::endl;
		std::exit(-1);
	}
}

std::string encrypt(const std::string &plaintext, const std::string &privatekey,
                    const std::string &publickey, std::string &nonce)
{
	nonce = std::string(crypto_box_NONCEBYTES, 0);
	randombytes((unsigned char*)&nonce[0], crypto_box_NONCEBYTES);
	return crypto_box(plaintext, nonce, publickey, privatekey);
}

std::string decrypt(const std::string &ciphertext, const std::string &privatekey,
                    const std::string &publickey, const std::string &nonce)
{
	return crypto_box_open(ciphertext, nonce, publickey, privatekey);
}

void printhelp(const std::string &name)
{
	std::cout << "usage: ./" << name << std::endl;
	std::cout << "-g generate private/ public key pair" << std::endl;
	std::cout << "-p specify public key file. Is created in combination with -g" << std::endl;
	std::cout << "-s specify secret key file. Is created in combination with -g" << std::endl;
	std::cout << "-h print help text" << std::endl;
}

int main(int argc, char *argv[])
{
	bool  gflag = false;
	char arg;
	std::string secretkeyfile="secret.key", publickeyfile="public.key";
	while((arg = getopt(argc,argv,"ghs:p:")) != -1)
		switch(arg) {
			case 'g':
				gflag = true;
				break;
			case 's':
				secretkeyfile = optarg;
				break;
			case 'p':
				publickeyfile = optarg;
				break;
			case 'h':
			default:
				printhelp(argv[0]);
				std::exit(0);
		}
	
	if (gflag) {
		generatekeys(secretkeyfile, publickeyfile);
		std::exit(0);
	}
	
	std::string privatekey, publickey;
	loadkeys(publickeyfile, secretkeyfile, publickey, privatekey);
	
	
	try{
		std::string nonce, message = "digane passwort";
		std::cout << "message: " << message << std::endl;
		std::string ciphertext = encrypt(message, privatekey, publickey, nonce);
		std::cout << "ciphertext: " << ciphertext << std::endl;
		std::string plaintext = decrypt(ciphertext, privatekey, publickey, nonce);
		std::cout << "plaintext: " << plaintext << std::endl;
	} catch(const char * e) {
		std::cout << e << std::endl;
	}
	return 0;
}
/*
https://github.com/simcop2387/underhanded-crypto/blob/master/Anonymous/submission/network.cpp
https://cr.yp.to/proto/netstrings.txt
*/