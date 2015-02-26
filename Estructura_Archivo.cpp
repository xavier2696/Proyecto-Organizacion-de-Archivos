#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <sstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

struct Data{
	string name;
	int type;
	int size;
};

struct Index{
	string key;
	int offset;
};

vector<Data*> ReadHeader(fstream* , int);
vector<Index*> ReadIndex(char* , int , Data*);

int main(int argc, char* argv[]){
	int opcion;
	bool index = false;
	do{
		cout<<"Menu"<<endl;
		if(index){
			cout<<"(Indices activados)"<<endl;
		}
		cout<<"1)Definir estructura y guardar datos"<<endl 
			<<"2)Agregar datos a archivo"<<endl
			<<"3)Listar datos de archivo"<<endl
			<<"4)Buscar Registro"<<endl
			<<"5)Borrar Registro"<<endl
			<<"6)Compactar"<<endl
			<<"7)Modificar Registro"<<endl;
		if(index){
			cout<<"8)Desactivar indices"<<endl;
		}else{
			cout<<"8)Activar indices"<<endl;
		}
		cout<<"9)Crear Arhivo indices"<<endl
			<<"10)Salir"<<endl
			<<"Ingrese su opcion: ";
		cin>>opcion;
		if(opcion == 1){
			//definicion de campos
			int continuar;
			vector<Data*> campos;
			cout<<"El primer campo que ingrese es el campo llave"<<endl;
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
			long int cant_registros = 0;
			out.write(reinterpret_cast<char*>(&cant_campos), sizeof(int));//numero de campos
			out.write(reinterpret_cast<char*>(&offset), sizeof(int));//avail list
			out.write(reinterpret_cast<char*>(&cant_registros), sizeof(long int));//espacio para cantidad de registros
			for(int i = 0; i<campos.size(); i++){
				for(int j =0 ; j<15; j++){
					out.write(reinterpret_cast<char*>(&((campos[i]->name)[j])), sizeof(char));
				}

				out.write(reinterpret_cast<char*>(&(campos[i]->type)), sizeof(int));
				out.write(reinterpret_cast<char*>(&(campos[i]->size)), sizeof(int));
			}
			out.close();


		}else if( opcion == 2){
			vector<Index*> indices;
			string ingresado;				
			stringstream ss;
			stringstream ss2;
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
				long int cant_registros;
				vector<Data*> campos;
				in.read(reinterpret_cast<char*>(&cant_campos),sizeof(int));
				in.read(reinterpret_cast<char*>(&avail_list),sizeof(int));
				in.read(reinterpret_cast<char*>(&cant_registros),sizeof(long int));
				campos = ReadHeader(&in,cant_campos);
				in.close();
				int disponible = avail_list;

				
				if(index){
					ss2<<ingresado<<".in";
					char* nombre_archivo2 = new char(ingresado.size() + 4);
					strcpy(nombre_archivo2, ss2.str().c_str());
					indices = ReadIndex(nombre_archivo2, cant_registros , campos[0]);
					//if(indices.size() == 0){
					//	cout<<"Ocurrio un error cuando se guardaron los indices. Debe reindexar antes de poder usar los indices."<<endl;
					//}
				}

				if(avail_list ==0){
					ofstream out(nombre_archivo, ios::out|ios::binary|ios::app);
					int continuar;
					do{
						int dato2;
						int offset2;
						for(int i = 0; i<campos.size(); i++){
							if(campos[i]->type == 1){
								int dato;								
								cout<<campos[i]->name<<": ";
								cin>>dato;
								if(i == 0){
									offset2 = out.tellp();
									dato2 = dato;
								}
								out.write(reinterpret_cast<char*>(&dato), sizeof(int));
								
							}else{
								if(campos[i]->size == 1){
									char dato;
									cout<<campos[i]->name<<": ";
									cin>>dato;
									out.write(reinterpret_cast<char*>(&dato),sizeof(char));
								}else{
									char* dato = new char[campos[i]->size];
									cout<<campos[i]->name<<": ";
									cin>>dato;
									for(int j = 0; j<campos[i]->size*sizeof(char); j++){
										out.write(reinterpret_cast<char*>(&(dato[j])),sizeof(char));
									}
								}
							}
						}
						cant_registros++;
						if(index){
							if(campos[0]->type == 1){								
								Index* indice = new Index;
								indice->offset = offset2;
								stringstream ss3;
								ss3<<dato2;
								indice->key = ss3.str();
								//int n = stoi(indice);
								int insert = 0;
								for(int i = 0; i<indices.size(); i++){
									if(atoi((indices[i]->key).c_str()) < atoi((indice->key).c_str())){
										insert++;
									}
								}
							//cout<<"pos: "<<insert<<endl;
								indices.insert(indices.begin()+insert, indice);	
							}else{

							}
						}
						cout<<"Ingrese 1 para ingresar otro registro: ";
						cin>>continuar;
					}while(continuar == 1);				
					out.close();
				}else{				
					int dato2;
					int offset2;
					int size_registro = 0;
					for(int i = 0; i<campos.size(); i++){
						if(campos[i]->type == 2)
							size_registro += campos[i]->size;
						else
							size_registro += sizeof(int);
					}

					int continuar;
					do{			
						ifstream in(nombre_archivo, ios::binary|ios::in);
						int offset_temp = sizeof(int)*2+sizeof(long int)+cant_campos*(sizeof(int)*2+15);
						offset_temp += (avail_list-1)*size_registro;
						offset_temp += sizeof(char);
						in.seekg(offset_temp, ios_base::beg);

						in.read(reinterpret_cast<char*>(&disponible),sizeof(int));
						in.close();			

						ofstream out(nombre_archivo, ios::out|ios::in|ios::binary);
						int offset = sizeof(int)*2+sizeof(long int)+cant_campos*(sizeof(int)*2+15);

						offset += (avail_list-1)*size_registro;
						out.seekp(offset, ios_base::beg);
						for(int i = 0; i<campos.size(); i++){
							if(campos[i]->type == 1){
								int dato;
								cout<<campos[i]->name<<": ";
								cin>>dato;
								//cin>>dato;
								if(i == 0){
									offset2 = out.tellp();
									dato2 = dato;
								}
								out.write(reinterpret_cast<char*>(&dato), sizeof(int));
								
							}else{
								if(campos[i]->size == 1){
									char dato;
									cout<<campos[i]->name<<": ";
									cin>>dato;
									out.write(reinterpret_cast<char*>(&dato),sizeof(char));
								}else{
									char* dato = new char[campos[i]->size];
									cout<<campos[i]->name<<": ";
									cin>>dato;
									for(int j = 0; j<campos[i]->size*sizeof(char); j++){
										out.write(reinterpret_cast<char*>(&(dato[j])),sizeof(char));
									}
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
						if(index){
							if(campos[0]->type == 1){								
								Index* indice = new Index;
								indice->offset = offset2;
								stringstream ss3;
								ss3<<dato2;
								indice->key = ss3.str();
								//int n = stoi(indice);
								int insert = 0;
								for(int i = 0; i<indices.size(); i++){
									if(atoi((indices[i]->key).c_str()) < atoi((indice->key).c_str())){
										insert++;
									}
								}
							//cout<<"pos: "<<insert<<endl;
								indices.insert(indices.begin()+insert, indice);	
							}else{

							}
						}

						out.close();
					}while(continuar == 1 && avail_list != 0);				

				}
				fstream out2(nombre_archivo, ios::out|ios::binary|ios::in);
				out2.seekp(sizeof(int), ios_base::beg);
				out2.write(reinterpret_cast<char*>(&avail_list), sizeof(int));//nuevo avail_list
				out2.write(reinterpret_cast<char*>(&cant_registros), sizeof(long int));//cantidad de registros
				//cout<<"cant registros 1 "<<cant_registros<<endl;
				out2.close();
				for(int i = 0; i<indices.size(); i++){
					cout<<indices[i]->key<<endl;
				}

			}

		}else if(opcion == 3){
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
				long int cant_registros;
				vector<Data*> campos;
				in.read(reinterpret_cast<char*>(&cant_campos),sizeof(int));
				in.read(reinterpret_cast<char*>(&avail_list),sizeof(int));
				in.read(reinterpret_cast<char*>(&cant_registros),sizeof(long int));
				campos = ReadHeader(&in,cant_campos);

				//cout<<"cant registros "<<cant_registros<<endl;
				for(int j = 0; j<cant_registros; j++){
					char asterisco = 'x';
					in.read(reinterpret_cast<char*>(&asterisco), sizeof(char));
					//cout<<asterisco<<endl;
					in.seekp(-sizeof(char),ios_base::cur);
					if(asterisco == '*'){
						//cout<<"*"<<endl;
						j--;
						for(int i = 0; i<campos.size(); i++){
							if(campos[i]->type == 1){
								in.seekp(sizeof(int), ios_base::cur);
							}else{
								in.seekp(sizeof(char)*campos[i]->size, ios_base::cur);
							}
						}
					}else{

						//cout<<"Registro "<<(j+1)<<endl;
						for(int i = 0; i<campos.size(); i++){			
							if(campos[i]->type == 1){
								int dato;
								in.read(reinterpret_cast<char*>(&dato), sizeof(int));
								cout<<campos[i]->name<<": "<<dato<<endl;
							}else{
								if(campos[i]->size == 1){
									char dato;
									in.read(&dato,sizeof(char));
									cout<<campos[i]->name<<": "<<dato<<endl;
								}else{
									char* dato = new char[campos[i]->size];
									in.read(dato, campos[i]->size*sizeof(char));

									cout<<campos[i]->name<<": "<<dato<<endl;
								}
							}
						}
					}
				}
				in.close();	
			}


		}else if(opcion == 4){
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
				long int cant_registros;
				vector<Data*> campos;
				in.read(reinterpret_cast<char*>(&cant_campos),sizeof(int));
				in.read(reinterpret_cast<char*>(&avail_list),sizeof(int));
				in.read(reinterpret_cast<char*>(&cant_registros),sizeof(long int));
				campos = ReadHeader(&in,cant_campos);
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
							if(campos[i]->size == 1){
								char dato;
								in.read(&dato,sizeof(char));
								cout<<campos[i]->name<<": "<<dato<<endl;
							}else{
								char* dato = new char[campos[i]->size];
								in.read(dato, campos[i]->size*sizeof(char));

								cout<<campos[i]->name<<": "<<dato<<endl;
							}
						}
					}
				}

			}	

		}else if(opcion == 5){
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
				in.close();
			}else{				
				int cant_campos;
				int avail_list;
				long int cant_registros;
				vector<Data*> campos;
				in.read(reinterpret_cast<char*>(&cant_campos),sizeof(int));
				in.read(reinterpret_cast<char*>(&avail_list),sizeof(int));
				in.read(reinterpret_cast<char*>(&cant_registros),sizeof(long int));
				campos = ReadHeader(&in,cant_campos);
				in.close();
				int offset = sizeof(int)*2 +sizeof(long int)+cant_campos*(sizeof(int)*2+15);
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
				cant_registros--;
				fstream out2(nombre_archivo, ios::out|ios::binary|ios::in);
				out2.seekp(sizeof(int), ios_base::beg);
				out2.write(reinterpret_cast<char*>(&avail_list), sizeof(int));//avail list
				out2.write(reinterpret_cast<char*>(&cant_registros), sizeof(long int));
				out2.close();

			}
		}else if(opcion == 6){
			string ingresado;				
			stringstream ss;
			stringstream ss2;
			cout<<"Ingrese el nombre del archivo para compactar: ";
			getline(cin,ingresado);
			getline(cin,ingresado);
			ss<<ingresado<<".bin";
			ss2<<ingresado<<"1.bin";
			char* nombre_archivo = new char(ingresado.size() + 5);
			strcpy(nombre_archivo, ss.str().c_str());
			char* nombre_archivo2 = new char(ingresado.size() + 6);
			strcpy(nombre_archivo2, ss2.str().c_str());
			fstream in(nombre_archivo, ios::in|ios::binary);
			fstream out(nombre_archivo2, ios::out|ios::binary);
			if(!in.is_open()){
				printf("El archivo no existe \n");
			}else{
				int cant_campos;
				int avail_list;
				long int cant_registros;
				vector<Data*> campos;
				in.read(reinterpret_cast<char*>(&cant_campos),sizeof(int));
				out.write(reinterpret_cast<char*>(&cant_campos),sizeof(int));
				in.read(reinterpret_cast<char*>(&avail_list),sizeof(int));
				out.write(reinterpret_cast<char*>(&avail_list),sizeof(int));
				in.read(reinterpret_cast<char*>(&cant_registros),sizeof(long int));
				out.write(reinterpret_cast<char*>(&cant_registros),sizeof(long int));
				//campos = ReadHeader(in,cant_campos);

				for(int i = 0; i<cant_campos; i++){
					Data* data = new Data;
					char* nombre = new char[15];
					int tipo;
					int size;
					in.read(nombre,sizeof(char)*15);
					for(int i = 0; i<15*sizeof(char); i++){
						out.write(reinterpret_cast<char*>(&(nombre[i])),sizeof(char));
					}

					nombre[14] = '\0';
					in.read(reinterpret_cast<char*>(&tipo),sizeof(int));
					out.write(reinterpret_cast<char*>(&tipo),sizeof(int));
					in.read(reinterpret_cast<char*>(&size),sizeof(int));
					out.write(reinterpret_cast<char*>(&size),sizeof(int));
					data->type = tipo;
					data->size = size;
					data->name = nombre;
					for(int i = 0; i<15; i++){
						nombre[i] = '\0';
					}				


					campos.push_back(data);

				}
				long int registros_temp = cant_registros;
				//cout<<"cant registros "<<cant_registros<<endl;
				for(int j = 0; j<cant_registros; j++){
					char asterisco = 'x';
					in.read(reinterpret_cast<char*>(&asterisco), sizeof(char));
					//cout<<asterisco<<endl;
					in.seekp(-sizeof(char),ios_base::cur);
					if(asterisco == '*'){
						registros_temp--;
						//cout<<"*"<<endl;
						for(int i = 0; i<campos.size(); i++){
							if(campos[i]->type == 1){
								in.seekp(sizeof(int), ios_base::cur);
							}else{
								in.seekp(sizeof(char)*campos[i]->size, ios_base::cur);
							}
						}
					}else{

						//cout<<"Registro "<<(j+1)<<endl;
						for(int i = 0; i<campos.size(); i++){			
							if(campos[i]->type == 1){
								int dato;
								in.read(reinterpret_cast<char*>(&dato), sizeof(int));
								out.write(reinterpret_cast<char*>(&dato), sizeof(int));
								//cout<<campos[i]->name<<": "<<dato<<endl;
							}else{
								if(campos[i]->size == 1){
									char dato;
									in.read(&dato, sizeof(char));
									out.write(reinterpret_cast<char*>(&dato),sizeof(char));
								}else{
									char* dato = new char[campos[i]->size];
									in.read(dato, campos[i]->size*sizeof(char));
									for(int i = 0; i<campos[i]->size*sizeof(char); i++){
										out.write(reinterpret_cast<char*>(&(dato[i])),sizeof(char));
									}
								}
								//out.write(dato, campos[i]->size*sizeof(char));

								//cout<<campos[i]->name<<": "<<dato<<endl;
							}
						}
					}
				}
				in.close();	
				out.close();
				remove(nombre_archivo);
				rename(nombre_archivo2,nombre_archivo);

				fstream out2(nombre_archivo, ios::out|ios::binary|ios::in);
				out2.seekp(sizeof(int), ios_base::beg);
				//cout<<"registros "<<cant_registros<<endl;
				int temp_avail = 0;
				out2.write(reinterpret_cast<char*>(&temp_avail), sizeof(int));//avail list
				out2.write(reinterpret_cast<char*>(&registros_temp), sizeof(long int));//cant registros
				out2.close();
			}
		}else if(opcion == 7){
			string ingresado;				
			stringstream ss;
			cout<<"Ingrese el nombre del archivo para modificar datos: ";
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
				long int cant_registros;
				vector<Data*> campos;
				in.read(reinterpret_cast<char*>(&cant_campos),sizeof(int));
				in.read(reinterpret_cast<char*>(&avail_list),sizeof(int));
				in.read(reinterpret_cast<char*>(&cant_registros),sizeof(long int));
				campos = ReadHeader(&in,cant_campos);
				in.close();

				int pos ;
				do{
					cout<<"Ingrese la posicion que desea modificar: ";
					cin>>pos;
				}while(pos<=0 && pos>cant_registros);

				int size_registro = 0;
				for(int i = 0; i<campos.size(); i++){
					if(campos[i]->type == 2)
						size_registro += campos[i]->size;
					else
						size_registro += sizeof(int);
				}
				ofstream out(nombre_archivo, ios::in|ios::out|ios::binary);
				int offset = sizeof(int)*2+sizeof(long int)+cant_campos*(sizeof(int)*2+15);

				offset += (pos-1)*size_registro;
				out.seekp(offset, ios_base::beg);
				for(int i = 0; i<campos.size(); i++){
					if(campos[i]->type == 1){
						int dato;
						cout<<campos[i]->name<<": ";
						cin>>dato;
						//cin>>dato;
						out.write(reinterpret_cast<char*>(&dato), sizeof(int));
					}else{
						if(campos[i]->size == 1){
							char dato;
							in.read(&dato, sizeof(char));
							out.write(reinterpret_cast<char*>(&dato),sizeof(char));
						}else{
							char* dato = new char[campos[i]->size];
							in.read(dato, campos[i]->size*sizeof(char));
							for(int i = 0; i<campos[i]->size*sizeof(char); i++){
								out.write(reinterpret_cast<char*>(&(dato[i])),sizeof(char));
							}
						}
					}
				}
				out.close();

			}

		}else if(opcion == 8){
			if(index)
				index = false;
			else
				index = true;

		}else if(opcion == 9){
			string ingresado;				
			stringstream ss;
			stringstream ss2;
			cout<<"Ingrese el nombre del archivo para re indexar los registros: ";
			getline(cin,ingresado);
			getline(cin,ingresado);
			ss<<ingresado<<".bin";
			ss2<<ingresado<<".in";
			char* nombre_archivo2 = new char(ingresado.size() + 4);
			char* nombre_archivo = new char(ingresado.size() + 5);
			strcpy(nombre_archivo, ss.str().c_str());
			strcpy(nombre_archivo2, ss2.str().c_str());
			nombre_archivo[ingresado.size()+4] = '\0';
			nombre_archivo2[ingresado.size()+3] = '\0';
			fstream in(nombre_archivo, ios::in|ios::binary);
			if(!in.is_open()){
				printf("El archivo no existe \n");
				in.close();
			}else{								
				int cant_campos;
				int avail_list;
				long int cant_registros;
				vector <Data*> campos;
				vector<Index*> indices;
				in.read(reinterpret_cast<char*>(&cant_campos),sizeof(int));
				in.read(reinterpret_cast<char*>(&avail_list),sizeof(int));
				in.read(reinterpret_cast<char*>(&cant_registros),sizeof(long int));
				campos = ReadHeader(&in,cant_campos);

				fstream out(nombre_archivo2, ios_base::out|ios_base::binary);
				for(int j = 0; j<cant_registros; j++){
					char asterisco = 'x';
					in.read(reinterpret_cast<char*>(&asterisco), sizeof(char));
					in.seekp(-sizeof(char),ios_base::cur);
					if(asterisco == '*'){
						for(int i = 0; i<campos.size(); i++){
							if(campos[i]->type == 1){
								in.seekp(sizeof(int), ios_base::cur);
							}else{
								in.seekp(sizeof(char)*campos[i]->size, ios_base::cur);
							}
						}
					}else{
						if(campos[0]->type == 1){
							int dato;
							int offset = in.tellg();
							in.read(reinterpret_cast<char*>(&dato), sizeof(int));
							Index* indice = new Index;
							indice->offset = offset;
							stringstream ss3;
							ss3<<dato;
							indice->key = ss3.str();
							//int n = stoi(indice);
							int insert = 0;
							for(int i = 0; i<indices.size(); i++){
								if(atoi((indices[i]->key).c_str()) < atoi((indice->key).c_str())){
									insert++;
								}
							}
							//cout<<"pos: "<<insert<<endl;
							indices.insert(indices.begin()+insert, indice);					
							
						}else{
							
						}


						for(int i = 1; i<campos.size(); i++){
							if(campos[i]->type == 1)
								in.seekp(sizeof(int),ios_base::cur);
							else
								in.seekp(campos[i]->size, ios_base::cur);
						}
						
					}
				}
				//for(int i = 0; i<indices.size(); i++){
				//				cout<<indices[i]->key<<endl;
				//			}
				in.close();
				char zero = '0';
				char uno = '1';	
				out.write(reinterpret_cast<char*>(&zero),sizeof(char));
				for(int i = 0; i<indices.size(); i++){
					if(campos[0]->type == 1){
						int temp = atoi((indices[i]->key).c_str());
						out.write(reinterpret_cast<char*>(&temp),sizeof(int));
						out.write(reinterpret_cast<char*>(&(indices[i]->offset)),sizeof(int));
					}else{

					}
				}
				out.seekg(0,ios_base::beg);
				out.write(reinterpret_cast<char*>(&uno),sizeof(char));
				out.close();
			}




		}else if(opcion == 10){

		}else{
			cout<<"La opcion no es valida"<<endl;

		}
	}while(opcion != 10);

	return 0;
}

vector<Data*> ReadHeader(fstream* in, int cant_campos){
	vector<Data*> campos;

	for(int i = 0; i<cant_campos; i++){
		Data* data = new Data;
		char* nombre = new char[15];
		int tipo;
		int size;
		in->read(nombre,sizeof(char)*15);
		nombre[14] = '\0';
		in->read(reinterpret_cast<char*>(&tipo),sizeof(int));
		in->read(reinterpret_cast<char*>(&size),sizeof(int));
		data->type = tipo;
		data->size = size;
		data->name = nombre;
		for(int i = 0; i<15; i++){
			nombre[i] = '\0';
		}



		campos.push_back(data);

	}
	return campos;
}

vector<Index*> ReadIndex(char* nombre_archivo, int cant_registros, Data* campo){
	vector<Index*> indices;
	ifstream in(nombre_archivo, ios::in|ios::binary);
	char seguro;
	in.read(reinterpret_cast<char*>(&seguro),sizeof(char));
	if(seguro == '0'){
		return indices;
	}
	else{
		for(int i = 0; i<cant_registros; i++){
			Index* indice = new Index;
			if(campo->type == 1){
				int key;
				int offset;
				in.read(reinterpret_cast<char*>(&key),sizeof(int));
				in.read(reinterpret_cast<char*>(&offset),sizeof(int));
				stringstream ss;
				ss<<key;
				indice->key = ss.str();
				indice->offset = offset;
				indices.push_back(indice);
			}else{

			}
		}
	}
	return indices;

}

