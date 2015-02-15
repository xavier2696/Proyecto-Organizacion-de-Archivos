#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <sstream>
#include <string>
#include <stdlib.h>
using namespace std;

struct Data{
	string name;
	int type;
	int size;
};


	int main(int argc, char* argv[]){
		char opcion;
		do{
			cout<<"Menu"<<endl 
				<<"1)Definir estructura y guardar datos"<<endl 
				<<"2)Agregar datos a archivo"<<endl
				<<"3)Listar datos de archivo"<<endl
				<<"4)Salir"<<endl 
				<<"Ingrese su opcion: ";
			cin>>opcion;
			if(opcion == '1'){
				//definicion de campos
				int continuar;
				vector<Data*> campos;
		
				do{
					Data* data = new Data;
					string nombre;
					cout<<"Ingrese el nombre del campo: ";
					cin>>nombre;
					data->name = nombre;		
					int tipo;
					do{
						cout<<"Ingrese 1 si es entero o 2 si es texto: ";
						cin>>tipo;
					}while(tipo != 1 && tipo != 2);
					data->type = tipo;
					if(tipo == 2){
						int longitud;
						do{
							cout<<"Ingrese la longitud maxima del texto: ";
							cin>>longitud;
						}while(longitud <= 0);
						data->size = longitud;
					}else if(tipo == 1){
						data->size = -1;
					}
					campos.push_back(data);
					cout<<"Ingrese 1 para ingresar otro campo: ";
					cin>>continuar;
				}while(continuar == 1);

				//introducir datos
				string ingresado;				
				stringstream ss;
				cout<<"Ingrese el nombre del archivo donde quiere guardar los datos: ";
				getline(cin,ingresado);
				getline(cin,ingresado);
				ss<<ingresado<<".bin";
				char* nombre_archivo = new char(ingresado.size() + 5);
				strcpy(nombre_archivo, ss.str().c_str());

				stringstream ss2;
				ss2<<ingresado<<".txt";
				char* nombre_archivo2 = new char(ingresado.size() + 5);
				strcpy(nombre_archivo2, ss2.str().c_str());

				ofstream out(nombre_archivo, ios::out|ios::binary);
				ofstream out2(nombre_archivo2, ios::out);

				int cant_registros = 0;
				do{
					for(int i = 0; i<campos.size(); i++){
						if(campos[i]->type == 1){
							int dato;
							cout<<campos[i]->name<<": ";
							cin>>dato;
							out.write(reinterpret_cast<char*>(&dato), sizeof(int));
						}else{
							char* dato = new char[campos[i]->size];
							cout<<campos[i]->name<<": ";
							cin>>dato;
							out.write(dato, campos[i]->size*sizeof(char));
						}
					}
					cant_registros++;
					cout<<"Ingrese 1 para ingresar otro registro: ";
					cin>>continuar;
				}while(continuar == 1);

				out2<<cant_registros<<endl;
				for(int i = 0; i<campos.size(); i++){
					out2<<campos[i]->name<<","<<campos[i]->type<<","<<campos[i]->size<<endl;

				}

				out.close();
				out2.close();
		}else if( opcion == '2'){
			string ingresado;				
			stringstream ss;
			cout<<"Ingrese el nombre del archivo para cargar datos: ";
			getline(cin,ingresado);
			getline(cin,ingresado);
			ss<<ingresado<<".bin";
			char* nombre_archivo = new char(ingresado.size() + 5);
			strcpy(nombre_archivo, ss.str().c_str());

			stringstream ss2;
			ss2<<ingresado<<".txt";
			char* nombre_archivo2 = new char(ingresado.size() + 5);
			strcpy(nombre_archivo2, ss2.str().c_str());

			ifstream f;
			f.open(nombre_archivo2,ios_base::in);
			if(!f.is_open()){
				printf("El archivo no existe \n");
			}else{
				ofstream out(nombre_archivo, ios::out|ios::binary|ios::app);
				string datos;
				getline(f,datos);
				int cant_registros;
				cant_registros = atoi(datos.c_str());
				vector<Data*> campos;
				string datos2;
				while(getline(f,datos2)){
					char datos3[datos2.size()+1];
					strcpy(datos3,datos2.c_str());
					char* pch = strtok(datos3,",");
					int pos = 0;
					Data* data = new Data;

					while (pch != NULL){  						
  						if(pos == 0){
  							data->name = pch;
  						}else if(pos == 1){
  							data->type = atoi(pch);
  						}else if (pos == 2){
  							data->size = atoi(pch);
  						}
    					pch = strtok (NULL, ",");
    					pos++;    					
  					}
  					campos.push_back(data);
				}
				int continuar;
				do{
					for(int i = 0; i<campos.size(); i++){
						if(campos[i]->type == 1){
							int dato;
							cout<<campos[i]->name<<": ";
							cin>>dato;
							out.write(reinterpret_cast<char*>(&dato), sizeof(int));
						}else{
							char* dato = new char[campos[i]->size];
							cout<<campos[i]->name<<": ";
							cin>>dato;
							out.write(dato, campos[i]->size*sizeof(char));
						}
					}
					cant_registros++;
					cout<<"Ingrese 1 para ingresar otro registro: ";
					cin>>continuar;
				}while(continuar == 1);

				out.close();
				f.close();

				ofstream out2(nombre_archivo2, ios::out);
				out2<<cant_registros<<endl;
				for(int i = 0; i<campos.size(); i++){
					out2<<campos[i]->name<<","<<campos[i]->type<<","<<campos[i]->size<<endl;

				}
				out2.close();
			}
		


		}else if(opcion == '3'){
			string ingresado;				
			stringstream ss;
			cout<<"Ingrese el nombre del archivo para mostrar datos: ";
			getline(cin,ingresado);
			getline(cin,ingresado);
			ss<<ingresado<<".bin";
			char* nombre_archivo = new char(ingresado.size() + 5);
			strcpy(nombre_archivo, ss.str().c_str());

			stringstream ss2;
			ss2<<ingresado<<".txt";
			char* nombre_archivo2 = new char(ingresado.size() + 5);
			strcpy(nombre_archivo2, ss2.str().c_str());

			ifstream f(nombre_archivo2);
			if(!f.is_open()){
				printf("El archivo no existe \n");
			}else{
				string registros;
				ifstream f(nombre_archivo2);
				getline(f,registros);
				int cant_registros = atoi(registros.c_str());
				string datos;
				vector<Data*> campos;

				while(getline(f,datos)){

					char datos2[datos.size()+1];
					strcpy(datos2,datos.c_str());
					datos2[datos.size()] = '\0';
					char* tk = strtok(datos2,",");
					char* datos_registro[3];
					
					int pos = 0;

					while(tk != NULL){
						datos_registro[pos] = tk;
						pos++;
						tk = strtok(NULL,",");
					}
					Data* data = new Data;
					data->name = datos_registro[0];
					data->type = atoi(datos_registro[1]);
					data->size = atoi(datos_registro[2]);
					campos.push_back(data);
				}
				
				fstream in(nombre_archivo, ios::in|ios::binary);
				for(int j = 0; j<cant_registros; j++){
					cout<<"Registro "<<(j+1)<<endl;
					for(int i = 0; i<campos.size(); i++){			
						if(campos[i]->type == 1){
							int dato;
							in.read(reinterpret_cast<char*>(&dato), sizeof(int));
							cout<<campos[i]->name<<": "<<dato<<endl;
						}else{
							char* dato = new char[campos[i]->size];
							in.read(dato, campos[i]->size*sizeof(char));
				
							cout<<campos[i]->name<<": "<<dato<<endl;
						}
					}
				}
				in.close();	
			}

			
		}else if(opcion == '4'){

		}else{
			cout<<"La opcion no es valida"<<endl;
		}
	}while(opcion != '4');
	
	return 0;
}