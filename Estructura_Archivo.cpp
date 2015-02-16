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
				 
				<<"4)Buscar Registro"<<endl
				<<"5)Borrar Registro"<<endl
				<<"6)Compactar"<<endl
				<<"7)Modificar Registro"<<endl
				<<"8)Salir"<<endl
				<<"Ingrese su opcion: ";
			cin>>opcion;
			if(opcion == '1'){
				//definicion de campos
				int continuar;
				vector<Data*> campos;
		
				do{
					Data* data = new Data;
					char* nombre = new char[15];
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

				ofstream out(nombre_archivo, ios::out|ios::binary);

				//escribir header
				int cant_campos = campos.size();
				int offset = 0;
				int cant_registros = 0;
				out.write(reinterpret_cast<char*>(&cant_campos), sizeof(int));//numero de campos
				out.write(reinterpret_cast<char*>(&offset), sizeof(int));//avail list
				out.write(reinterpret_cast<char*>(&cant_registros), sizeof(int));//espacio para cantidad de registros
				for(int i = 0; i<campos.size(); i++){
					for(int j =0 ; j<15; j++){
						out.write(reinterpret_cast<char*>(&((campos[i]->name)[j])), sizeof(char));
					}
					
					out.write(reinterpret_cast<char*>(&(campos[i]->type)), sizeof(int));
					out.write(reinterpret_cast<char*>(&(campos[i]->size)), sizeof(int));
				}
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
							for(int j = 0; j<campos[i]->size*sizeof(char); j++){
								out.write(reinterpret_cast<char*>(&(dato[j])),sizeof(char));
							}
						}
					}
					cant_registros++;
					cout<<"Ingrese 1 para ingresar otro registro: ";
					cin>>continuar;
				}while(continuar == 1);

				
  				out.seekp (sizeof(int)*2, ios::beg);
  				out.write(reinterpret_cast<char*>(&cant_registros), sizeof(int));//cantidad de registros
				out.close();
		}else if( opcion == '2'){
			string ingresado;				
			stringstream ss;
			cout<<"Ingrese el nombre del archivo para cargar datos: ";
			getline(cin,ingresado);
			getline(cin,ingresado);
			ss<<ingresado<<".bin";
			char* nombre_archivo = new char(ingresado.size() + 5);
			strcpy(nombre_archivo, ss.str().c_str());

			
			fstream in(nombre_archivo, ios::in|ios::binary);
			if(!in.is_open()){
				printf("El archivo no existe \n");
			}else{
				
				int cant_campos;
				int avail_list;
				int cant_registros;
				vector<Data*> campos;
				in.read(reinterpret_cast<char*>(&cant_campos),sizeof(int));
				in.read(reinterpret_cast<char*>(&avail_list),sizeof(int));
				in.read(reinterpret_cast<char*>(&cant_registros),sizeof(int));
				
				for(int i = 0; i<cant_campos; i++){
					Data* data = new Data;
					char* nombre = new char[15];
					int tipo;
					int size;
					in.read(nombre,sizeof(char)*15);
					nombre[14] = '\0';
					in.read(reinterpret_cast<char*>(&tipo),sizeof(int));
					in.read(reinterpret_cast<char*>(&size),sizeof(int));
					data->type = tipo;
					data->size = size;
					data->name = nombre;
					for(int i = 0; i<15; i++){
						nombre[i] = '\0';
					}



					campos.push_back(data);

				}
				in.close();
				int disponible = avail_list;
				if(avail_list ==0){
					ofstream out(nombre_archivo, ios::out|ios::binary|ios::app);
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
								for(int j = 0; j<campos[i]->size*sizeof(char); j++){
									out.write(reinterpret_cast<char*>(&(dato[j])),sizeof(char));
								}
							}
						}
						cant_registros++;
						cout<<"Ingrese 1 para ingresar otro registro: ";
						cin>>continuar;
					}while(continuar == 1);				
					out.close();
				}else{				

					int size_registro = 0;
					for(int i = 0; i<campos.size(); i++){
						if(campos[i]->type == 2)
							size_registro += campos[i]->size;
						else
							size_registro += sizeof(int);
					}

					/*ifstream in(nombre_archivo, ios::binary|ios::in);
					int offset_temp = sizeof(int)*3+cant_campos*(sizeof(int)*2+15);
					offset_temp += (avail_list-1)*size_registro;
					offset_temp += sizeof(char);
					in.seekg(offset_temp, ios_base::beg);
					
					in.read(reinterpret_cast<char*>(&disponible),sizeof(int));
					in.close();*/

					
					int continuar;
					do{			
						ifstream in(nombre_archivo, ios::binary|ios::in);
						int offset_temp = sizeof(int)*3+cant_campos*(sizeof(int)*2+15);
						offset_temp += (avail_list-1)*size_registro;
						offset_temp += sizeof(char);
						in.seekg(offset_temp, ios_base::beg);

						in.read(reinterpret_cast<char*>(&disponible),sizeof(int));
						in.close();			

						ofstream out(nombre_archivo, ios::out|ios::in|ios::binary);
						int offset = sizeof(int)*3+cant_campos*(sizeof(int)*2+15);
						
						offset += (avail_list-1)*size_registro;
					    out.seekp(offset, ios_base::beg);
					    for(int i = 0; i<campos.size(); i++){
							if(campos[i]->type == 1){
								int dato;
								cout<<campos[i]->name<<": ";
								cin>>dato;
								//cin>>dato;
								out.write(reinterpret_cast<char*>(&dato), sizeof(int));
							}else{
								char* dato = new char[campos[i]->size];
								cout<<campos[i]->name<<": ";
								cin>>dato;
								//cin>>dato;
								for(int j = 0; j<campos[i]->size*sizeof(char); j++){
									out.write(reinterpret_cast<char*>(&(dato[j])),sizeof(char));
								}
							}
						}
						if(disponible == 0){
							cout<<"Vuelva a seleccionar agregar para seguir agregando registros"<<endl;
							continuar = 0;
						}else{
							cout<<"Ingrese 1 para ingresar otro registro: ";
							cin>>continuar;
						}
						//cant_registros++;
						avail_list = disponible;

						
						out.close();
					}while(continuar == 1 && avail_list != 0);				
					
				}
				fstream out2(nombre_archivo, ios::out|ios::binary|ios::in);
				out2.seekp(sizeof(int), ios_base::beg);
				out2.write(reinterpret_cast<char*>(&avail_list), sizeof(int));//nuevo avail_list
  				out2.write(reinterpret_cast<char*>(&cant_registros), sizeof(int));//cantidad de registros
  				//cout<<"cant registros 1 "<<cant_registros<<endl;
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
			fstream in(nombre_archivo, ios::in|ios::binary);
			if(!in.is_open()){
				printf("El archivo no existe \n");
			}else{
				int cant_campos;
				int avail_list;
				int cant_registros;
				vector<Data*> campos;
				in.read(reinterpret_cast<char*>(&cant_campos),sizeof(int));
				in.read(reinterpret_cast<char*>(&avail_list),sizeof(int));
				in.read(reinterpret_cast<char*>(&cant_registros),sizeof(int));
				
				for(int i = 0; i<cant_campos; i++){
					Data* data = new Data;
					char* nombre = new char[15];
					int tipo;
					int size;
					in.read(nombre,sizeof(char)*15);
					nombre[14] = '\0';
					in.read(reinterpret_cast<char*>(&tipo),sizeof(int));
					in.read(reinterpret_cast<char*>(&size),sizeof(int));
					data->type = tipo;
					data->size = size;
					data->name = nombre;
					for(int i = 0; i<15; i++){
						nombre[i] = '\0';
					}				
					
					
					campos.push_back(data);

				}
				
				cout<<"cant registros "<<cant_registros<<endl;
				for(int j = 0; j<cant_registros; j++){
					char asterisco = 'x';
					in.read(reinterpret_cast<char*>(&asterisco), sizeof(char));
					//cout<<asterisco<<endl;
					in.seekp(-sizeof(char),ios_base::cur);
					if(asterisco == '*'){
						//cout<<"*"<<endl;
						for(int i = 0; i<campos.size(); i++){
							if(campos[i]->type == 1){
								in.seekp(sizeof(int), ios_base::cur);
							}else{
								in.seekp(sizeof(char)*campos[i]->size, ios_base::cur);
							}
						}
					}else{

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
				}
				in.close();	
			}

			
		}else if(opcion == '4'){
			string ingresado;				
			stringstream ss;
			cout<<"Ingrese el nombre del archivo para mostrar los registros: ";
			getline(cin,ingresado);
			getline(cin,ingresado);
			ss<<ingresado<<".bin";
			char* nombre_archivo = new char(ingresado.size() + 5);
			strcpy(nombre_archivo, ss.str().c_str());
			fstream in(nombre_archivo, ios::in|ios::binary);
			if(!in.is_open()){
				printf("El archivo no existe \n");
			}else{
				
				int cant_campos;
				int avail_list;
				int cant_registros;
				vector<Data*> campos;
				in.read(reinterpret_cast<char*>(&cant_campos),sizeof(int));
				in.read(reinterpret_cast<char*>(&avail_list),sizeof(int));
				in.read(reinterpret_cast<char*>(&cant_registros),sizeof(int));
				
				for(int i = 0; i<cant_campos; i++){
					Data* data = new Data;
					char* nombre = new char[15];
					int tipo;
					int size;
					in.read(nombre,sizeof(char)*15);
					nombre[14] = '\0';
					in.read(reinterpret_cast<char*>(&tipo),sizeof(int));
					in.read(reinterpret_cast<char*>(&size),sizeof(int));
					data->type = tipo;
					data->size = size;
					data->name = nombre;
					for(int i = 0; i<15; i++){
						nombre[i] = '\0';
					}				
					
					
					campos.push_back(data);

				}
				int pos;
				do{
					cout<<"Ingrese la posicion de registro que desea desplegar: ";
					cin>>pos;
				}while(pos<0 && pos>cant_registros);
				int tam_registro = 0;
				char asterisco = 'x';
				in.read(reinterpret_cast<char*>(&asterisco), sizeof(char));
				//cout<<asterisco<<endl;

				in.seekp(-sizeof(char),ios_base::cur);

				if(asterisco == '*'){
					cout<<"Esa posicion fue borrada. Por favor seleccione otra."<<endl;
				}else{
					for(int i = 0; i<campos.size(); i++){
						if(campos[i]->type == 1){
							tam_registro+= sizeof(int);
						}else{
							tam_registro+= campos[i]->size;
						}
					}
					in.seekp((pos-1)*tam_registro,ios_base::cur);
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

			}	

		}else if(opcion == '5'){
			string ingresado;				
			stringstream ss;
			cout<<"Ingrese el nombre del archivo para borrar el registro: ";
			getline(cin,ingresado);
			getline(cin,ingresado);
			ss<<ingresado<<".bin";
			char* nombre_archivo = new char(ingresado.size() + 5);
			strcpy(nombre_archivo, ss.str().c_str());
			fstream in(nombre_archivo, ios::in|ios::binary);
			if(!in.is_open()){
				printf("El archivo no existe \n");
			}else{				
				int cant_campos;
				int avail_list;
				int cant_registros;
				vector<Data*> campos;
				in.read(reinterpret_cast<char*>(&cant_campos),sizeof(int));
				in.read(reinterpret_cast<char*>(&avail_list),sizeof(int));
				in.read(reinterpret_cast<char*>(&cant_registros),sizeof(int));
				for(int i = 0; i<cant_campos; i++){
					Data* data = new Data;
					char* nombre = new char[15];
					int tipo;
					int size;
					in.read(nombre,sizeof(char)*15);
					nombre[14] = '\0';
					in.read(reinterpret_cast<char*>(&tipo),sizeof(int));
					in.read(reinterpret_cast<char*>(&size),sizeof(int));
					data->type = tipo;
					data->size = size;
					data->name = nombre;
					for(int i = 0; i<15; i++){
						nombre[i] = '\0';
					}				
					
					
					campos.push_back(data);

				}
				in.close();
				int offset = sizeof(int)*3+cant_campos*(sizeof(int)*2+15);
				int size_registro = 0;
				int pos;
				do{
					cout << "Hay "<<cant_registros<<" registros. Ingrese la posicion de registro que desea borrar: ";
					cin>>pos;
				}while(pos <=0 && pos>cant_registros);				
				for(int i = 0; i<campos.size(); i++){
					if(campos[i]->type == 2)
						size_registro += campos[i]->size;
					else
						size_registro += sizeof(int);
				}
				offset += (pos-1)*size_registro;
				ofstream out(nombre_archivo, ios::out|ios::binary|ios::in);
				out.seekp(offset,ios_base::beg);
				char asterisco = '*';
				out.write(reinterpret_cast<char*>(&asterisco),sizeof(char));
				out.write(reinterpret_cast<char*>(&avail_list),sizeof(int));
				avail_list = pos;
				out.close();

				fstream out2(nombre_archivo, ios::out|ios::binary|ios::in);
				out2.seekp(sizeof(int), ios_base::beg);
  				out2.write(reinterpret_cast<char*>(&avail_list), sizeof(int));//avail list
  				out2.close();
				/*char primero;
				bool borrado = false;
				int ultimo_borrado;
				do{
					char primero;
					in.read(reinterpret_cast<char*>(&primero),sizeof(char));
					if(primero == '*'){
						in.read(reinterpret_cast<char*>(&ultimo_borrado),sizeof(int))
					}else{

					}
				}while(borrado);*/

			}
		}else if(opcion == '6'){

		}else if(opcion == '7'){

		}else if(opcion == '8'){

		}else{
			cout<<"La opcion no es valida"<<endl;
		
		}
	}while(opcion != '8');
	
	return 0;
}