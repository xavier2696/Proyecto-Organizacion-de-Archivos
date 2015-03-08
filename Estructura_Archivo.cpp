#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <sstream>
#include <string>
#include <stdlib.h>
#include <map>
#include <stdexcept> 
#include <iomanip>
using namespace std;

struct Data{
	string name;
	int type;
	int size;
};

vector<Data*> ReadHeader(fstream* , int);
bool ReadIndex(map<string,int>* ,char* ,long int , int , int);
void Reindexar(fstream*, char*);
bool Repetido(char* ,string,long int ,vector<Data*> );

int main(int argc, char* argv[]){
	int opcion;
	bool index = false;
	do{
		cout<<"Menu"<<endl;

		if(index){
			cout<<"(Indices activados)"<<endl;
		}
		cout<<"1)Definir estructura "<<endl 
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
		cout<<"9)Crear archivo de indices"<<endl
			<<"10)Salir"<<endl
			<<"Ingrese su opcion: ";
		cin>>opcion;
		if(opcion == 1){
			//definicion de campos
			int continuar;
			vector<Data*> campos;
			Data* borrado = new Data;
			borrado->name = "borrado";
			borrado->type = 2;
			borrado->size = 1;
			campos.push_back(borrado);
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
			int cant_registros = 0;
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
			string ingresado;				
			stringstream ss;
			cout<<"Ingrese el nombre del archivo para cargar datos: ";
			getline(cin,ingresado);
			getline(cin,ingresado);
			ss<<ingresado<<".bin";
			char* nombre_archivo = new char(ingresado.size() + 5);
			strcpy(nombre_archivo, ss.str().c_str());
			stringstream ss2;
			ss2<<ingresado<<".in";
			char* nombre_archivo2 = new char(ingresado.size() + 4);
			strcpy(nombre_archivo2, ss2.str().c_str());
			nombre_archivo2[ingresado.size()+3] = '\0';

			fstream in(nombre_archivo, ios::in|ios::binary);
			if(!in.is_open()){
				printf("El archivo no existe \n");
			}else{

				int cant_campos;
				int avail_list;
				long int cant_registros;
				vector<Data*> campos;
				map<string,int> indices;
				in.read(reinterpret_cast<char*>(&cant_campos),sizeof(int));
				in.read(reinterpret_cast<char*>(&avail_list),sizeof(int));
				in.read(reinterpret_cast<char*>(&cant_registros),sizeof(long int));
				campos = ReadHeader(&in,cant_campos);
				in.close();
				int disponible = avail_list;
				string dato2;
				int offset;
				string llave_ingresada;
				
				
				if(avail_list ==0){
					ofstream out(nombre_archivo, ios::out|ios::binary|ios::app);
					int continuar;

					do{
						int cont = 0;
						do{	
							if(cont != 0)
								cout<<"La llave que ingreso ya existe, por favor ingrese una no repetida."<<endl;					
							cout<<campos[1]->name<<": ";
							cin>>llave_ingresada;
							cont++;
						}while(Repetido(nombre_archivo,llave_ingresada,cant_registros,campos));
						char uno = '1';
						out.write(reinterpret_cast<char*>(&uno), sizeof(char));
						for(int i = 1; i<campos.size(); i++){
							if(campos[i]->type == 1){
								int dato;
								if( i ==1){
									//stringstream ss;
									//ss<<dato;
									dato2 = llave_ingresada;
									offset = out.tellp();
									offset--;
									dato = atoi(llave_ingresada.c_str());
								}else{
									cout<<campos[i]->name<<": ";
									cin>>dato;
								}
								out.write(reinterpret_cast<char*>(&dato), sizeof(int));
							}else{
								if(campos[i]->size == 1){
									char dato;
									cout<<campos[i]->name<<": ";
									cin>>dato;
									if(i == 1){
										//stringstream ss;
										//ss<<dato;
										dato2 = llave_ingresada;
										offset = out.tellp();
										offset--;
										dato = llave_ingresada[0];
									}else{

									}
									out.write(reinterpret_cast<char*>(&dato),sizeof(char));
								}else{
									char* dato = new char[campos[i]->size];									
									if(i ==1){
										dato2 = llave_ingresada;
										offset = out.tellp();
										offset--;
										strcpy(dato,llave_ingresada.c_str());
									}else{
										cout<<campos[i]->name<<": ";
										cin>>dato;
									}
									for(int j = 0; j<campos[i]->size*sizeof(char); j++){
										out.write(reinterpret_cast<char*>(&(dato[j])),sizeof(char));
									}
								}
							}
						}
						cant_registros++;

						if(index){
							if (!ReadIndex(&indices,nombre_archivo2,cant_registros-1, campos[1]->type, campos[1]->type)){
								cout<<"El archivo de indices no esta actualizado o no se guardo correctamente."<<endl
									<<"Porfavor reindexe el archivo antes de continuar"<<endl;
								break;
							}else{
								indices.insert(pair<string,int>(dato2,offset));
							}
						}
						out.flush();
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

					int continuar;
					do{			
						int cont = 0;
						do{	
							if(cont != 0)
								cout<<"La llave que ingreso ya existe, por favor ingrese una no repetida."<<endl;					
							cout<<campos[1]->name<<": ";
							cin>>llave_ingresada;
							cont++;
						}while(Repetido(nombre_archivo,llave_ingresada,cant_registros,campos));
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
						char uno = '1';
						out.write(reinterpret_cast<char*>(&uno), sizeof(char));
						for(int i = 1; i<campos.size(); i++){
							if(campos[i]->type == 1){
								int dato;								
								if(i ==1){
									//stringstream ss;
									//ss<<dato;
									dato2 = llave_ingresada;
									offset = out.tellp();
									offset--;
									dato = atoi(llave_ingresada.c_str());
								}else{
									cout<<campos[i]->name<<": ";
									cin>>dato;
								}
								out.write(reinterpret_cast<char*>(&dato), sizeof(int));
							}else{
								if(campos[i]->size == 1){
									char dato;
									
									if(i == 1){
										//stringstream ss;
										//ss<<dato;
										dato2 = llave_ingresada;
										offset = out.tellp();
										offset--;
										dato = llave_ingresada[0];
									}else{
										cout<<campos[i]->name<<": ";
										cin>>dato;
									}
									out.write(reinterpret_cast<char*>(&dato),sizeof(char));
								}else{
									char* dato = new char[campos[i]->size];
									
									if(i==1){
										dato2 = llave_ingresada;
										offset = out.tellp();
										offset--;
										strcpy(dato,llave_ingresada.c_str());
									}else{
										cout<<campos[i]->name<<": ";
										cin>>dato;
									}
									for(int j = 0; j<campos[i]->size*sizeof(char); j++){
										out.write(reinterpret_cast<char*>(&(dato[j])),sizeof(char));
									}
								}
							}
						}
						out.flush();
						if(disponible == 0){
							cout<<"Vuelva a seleccionar agregar para seguir agregando registros"<<endl;
							continuar = 0;
						}else{
							cout<<"Ingrese 1 para ingresar otro registro: ";
							cin>>continuar;
						}
						//cant_registros++;
						avail_list = disponible;

						cant_registros++;

						if(index){
							if (!ReadIndex(&indices,nombre_archivo2,cant_registros, campos[1]->type, campos[1]->type)){
								cout<<"El archivo de indices no esta actualizado o no se guardo correctamente."<<endl
									<<"Porfavor reindexe el archivo antes de continuar."<<endl;
								break;
							}else{
								indices.insert(pair<string,int>(dato2,offset));
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
				if(index){
					fstream out(nombre_archivo2, ios_base::out|ios_base::binary);
					char terminado = '0';
					out.write(reinterpret_cast<char*>(&terminado), sizeof(char));
					out.write(reinterpret_cast<char*>(&cant_registros), sizeof(long int));
					typedef map<string, int>::const_iterator MapIterator;
					for (MapIterator x = indices.begin(); x != indices.end(); x++) {
						//cout << x->first << ": " << x->second << '\n';
						if(campos[1]->type == 1){
							int key = atoi((x->first).c_str());
							int offset = x->second;
							out.write(reinterpret_cast<char*>(&key), sizeof(int));
							out.write(reinterpret_cast<char*>(&offset), sizeof(int));
						}else{
							if(campos[1]->size == 1){
								char key = (x->first)[0];
								char offset = x->second;
								out.write(reinterpret_cast<char*>(&key), sizeof(char));
								out.write(reinterpret_cast<char*>(&offset), sizeof(int));
							}else{
								char* key = new char[campos[1]->size];
								strcpy(key,(x->first).c_str());
								char offset = x->second;
								out.write(key, sizeof(char)*campos[1]->size);
								out.write(reinterpret_cast<char*>(&offset), sizeof(int));
							}
						}
					}
					out.seekg(0,ios_base::beg);
					terminado = '1';
					out.write(reinterpret_cast<char*>(&terminado), sizeof(char));
					out.close();

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
				int sizeint = 10;
				for(int k = 1; k<campos.size(); k++){
					if(campos[k]->type == 1){
						if(k == 1)
							cout<<setw(sizeint+19)<<right<<campos[k]->name;
						else
							cout<<setw(sizeint)<<campos[k]->name;
						//cout<<campos[i]->name;
					}else{
						if(k == 1)
							cout<<setw(campos[k]->size+15+19)<<campos[k]->name;
						else
							cout<<setw(campos[k]->size+15)<<campos[k]->name;
						//cout<<campos[i]->name;
					}
				}
				cout<<endl;
				if(!index){
					for(int j = 0; j<cant_registros; j++){
						char borrado;
						in.read(reinterpret_cast<char*>(&borrado), sizeof(char));
						if(borrado == '0'){
							for(int i = 1; i<campos.size(); i++){
								if(campos[i]->type == 1){
									in.seekp(sizeof(int), ios_base::cur);
								}else{
									in.seekp(sizeof(char)*campos[i]->size, ios_base::cur);
								}
							}
							j--;
						}else{

							cout<<"Registro "<<setw(10)<<left<<(j+1);
							for(int i = 1; i<campos.size(); i++){

								if(campos[i]->type == 1){
									int dato;
									in.read(reinterpret_cast<char*>(&dato), sizeof(int));
									//cout<<campos[i]->name<<": "<<dato<<endl;
									cout<<setw(sizeint)<<right<<dato;
								}else{
									if(campos[i]->size == 1){
										char dato;
										in.read(&dato,sizeof(char));
										//cout<<campos[i]->name<<": "<<dato<<endl;
										cout<<setw(campos[i]->size+15)<<right<<dato;
									}else{
										char* dato = new char[campos[i]->size];
										in.read(dato, campos[i]->size*sizeof(char));

										//cout<<campos[i]->name<<": "<<dato<<endl;
										cout<<setw(campos[i]->size+15)<<right<<dato;
									}
								}
								
							}
							cout<<endl;
						}
						if(j%10 == 0 && j!=0){
							char respuesta2;
							cout<<"Ingrese 1 para ver 10 registros mas: ";
							cin>>respuesta2;
							if(respuesta2 != '1')
								break;
						}
					}
				}else{
					map<string,int> indices1;
					map<int,int> indices;
					stringstream ss2;
					ss2<<ingresado<<".in";
					char* nombre_archivo2 = new char(ingresado.size() + 4);
					strcpy(nombre_archivo2, ss2.str().c_str());
					nombre_archivo2[ingresado.size()+3] = '\0';
					if (!ReadIndex(&indices1,nombre_archivo2,cant_registros, campos[1]->type, campos[1]->size)){
						cout<<"El archivo de indices no esta actualizado o no se guardo correctamente."<<endl
							<<"Porfavor reindexe el archivo antes de continuar"<<endl;
						//break;
					}else{
						typedef map<string, int>::const_iterator MapIterator;
						for (MapIterator x = indices1.begin(); x != indices1.end(); x++) {
							indices.insert(pair<int,int>(atoi((x->first).c_str()),x->second));

						}
						typedef map<int, int>::const_iterator MapIterator2;
						int cont = 1;
						for (MapIterator2 x = indices.begin(); x != indices.end(); x++) {
							in.seekg(x->second+1,ios_base::beg);
							cout<<"Registro "<<setw(10)<<left<<cont;
							cont++;
							for(int i = 1; i<campos.size(); i++){
								if(campos[i]->type == 1){
									int dato;
									in.read(reinterpret_cast<char*>(&dato), sizeof(int));
									//cout<<campos[i]->name<<": "<<dato<<endl;
									cout<<setw(sizeint)<<right<<dato;
								}else{
									if(campos[i]->size == 1){
										char dato;
										in.read(&dato,sizeof(char));
										//cout<<campos[i]->name<<": "<<dato<<endl;
										cout<<setw(campos[i]->size+15)<<right<<dato;
									}else{
										char* dato = new char[campos[i]->size];
										in.read(dato, campos[i]->size*sizeof(char));
										cout<<setw(campos[i]->size+15)<<right<<dato;
										//cout<<campos[i]->name<<": "<<dato<<endl;
									}
								}
							}
							cout<<endl;
							if(cont%10 == 0 && cont!=0){
								char respuesta2;
								cout<<"Ingrese 1 para ver 10 registros mas: ";
								cin>>respuesta2;
								if(respuesta2 != '1')
									break;
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
			stringstream ss2;
			ss2<<ingresado<<".in";
			char* nombre_archivo2 = new char(ingresado.size() + 4);
			strcpy(nombre_archivo2, ss2.str().c_str());
			nombre_archivo2[ingresado.size()+3] = '\0';
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
				string key;
				cout<<"Ingrese la llave de registro que desea desplegar: ";
				cin>>key;
				int tam_registro = 0;
				for(int i = 0; i<campos.size(); i++){
					if(campos[i]->type == 1){
						tam_registro+= sizeof(int);
					}else{
						tam_registro+= campos[i]->size;
					}
				}
				char borrado;
				int posicion = -1;
				if(!index){
					for(int i = 0; i<cant_registros; i++){
						in.read(reinterpret_cast<char*>(&borrado), sizeof(char));
						if(borrado == '0'){
							in.seekp(tam_registro, ios_base::cur);
						}else{
							if(campos[1]->type == 1){
								int dato;
								in.read(reinterpret_cast<char*>(&dato), sizeof(int));
								stringstream ss;
								ss<<dato;
								if(ss.str() == key){
									posicion = in.tellg();
									posicion -= sizeof(int);
									break;
								}
							}else{
								if(campos[1]->size == 1){
									char dato;
									in.read(reinterpret_cast<char*>(&dato), sizeof(char));
									stringstream ss;
									ss<<dato;
									if(ss.str() == key){
										posicion = in.tellg();
										posicion -= sizeof(char);
										break;
									}
								}else{
									char* dato;
									in.read(dato,sizeof(char)*campos[1]->size);
									string ss = string(dato);
									if(dato == key){
										posicion = in.tellg();
										posicion -= sizeof(char)*campos[1]->size;
										break;
									}
								}
							}
						}
						for(int i = 2; i<campos.size(); i++){
							if(campos[i]->type == 1){
								in.seekp(sizeof(int),ios_base::cur);
							}else{
								in.seekp(sizeof(char)*campos[i]->size,ios_base::cur);
							}
						}
					}
				}else{
					map<string,int> indices;
					if (!ReadIndex(&indices,nombre_archivo2,cant_registros, campos[1]->type, campos[1]->type)){
						cout<<"El archivo de indices no esta actualizado o no se guardo correctamente."<<endl
							<<"Porfavor reindexe el archivo antes de continuar"<<endl;
					}else{
						try{
							posicion = indices.at(key);
							posicion++;
						}catch(const std::out_of_range& oor){
							posicion = -1;
						}
					}
				}
				if(posicion == -1){
					cout<<"No se encontro un registro con esa llave"<<endl;
				}else{
					in.seekp(posicion,ios_base::beg);
					for(int i = 1; i<campos.size(); i++){			
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
			stringstream ss2;
			ss2<<ingresado<<".in";
			char* nombre_archivo2 = new char(ingresado.size() + 4);
			strcpy(nombre_archivo2, ss2.str().c_str());
			nombre_archivo2[ingresado.size()+3] = '\0';
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
				map<string,int> indices;
				int pos = 0;
				int offset2 = sizeof(int)*2 +sizeof(long int)+cant_campos*(sizeof(int)*2+15);
				string key;
				cout<<"Ingrese la llave de registro que desea eliminar: ";
				cin>>key;
				int tam_registro = 0;
				for(int i = 0; i<campos.size(); i++){
					if(campos[i]->type == 1){
						tam_registro+= sizeof(int);
					}else{
						tam_registro+= campos[i]->size;
					}
				}
				char borrado;
				int posicion = -1;
				if(!index){
					for(int i = 0; i<cant_registros; i++){
						in.read(reinterpret_cast<char*>(&borrado), sizeof(char));
						if(borrado == '0'){
							in.seekp(tam_registro, ios_base::cur);
						}else{
							if(campos[1]->type == 1){
								int dato;
								in.read(reinterpret_cast<char*>(&dato), sizeof(int));
								stringstream ss;
								ss<<dato;
								if(ss.str() == key){
									posicion = in.tellg();
									posicion -= sizeof(int);
									break;
								}
							}else{
								if(campos[1]->size == 1){
									char dato;
									in.read(reinterpret_cast<char*>(&dato), sizeof(char));
									stringstream ss;
									ss<<dato;
									if(ss.str() == key){
										posicion = in.tellg();
										posicion -= sizeof(char);
										break;
									}
								}else{
									char* dato;
									in.read(dato,sizeof(char)*campos[1]->size);
									string ss = string(dato);
									if(dato == key){
										posicion = in.tellg();
										posicion -= sizeof(char)*campos[1]->size;
										break;
									}
								}
							}
						}
						for(int i = 2; i<campos.size(); i++){
							if(campos[i]->type == 1){
								in.seekp(sizeof(int),ios_base::cur);
							}else{
								in.seekp(sizeof(char)*campos[i]->size,ios_base::cur);
							}
						}
					}
				}else{

					if (!ReadIndex(&indices,nombre_archivo2,cant_registros, campos[1]->type, campos[1]->type)){
						cout<<"El archivo de indices no esta actualizado o no se guardo correctamente."<<endl
							<<"Porfavor reindexe el archivo antes de continuar"<<endl;
					}else{
						try{
							posicion = indices.at(key);
							posicion++;

						}catch(const std::out_of_range& oor){
							posicion = -1;
						}
					}
				}
				if(posicion == -1){
					cout<<"No se encontro un registro con esa llave"<<endl;
					if(index){
						cout<<"Puede ser que el archivo de indices no esta actualizado. Intente reindexar"<<endl;
					}
				}else{
					//indices.erase(key_borrar);
					int pos = (posicion-1-offset2)/tam_registro +1;	
					cant_registros--;
					if(index){
						indices.erase(key);
						fstream out(nombre_archivo2, ios_base::out|ios_base::binary);
						char terminado = '0';

						out.write(reinterpret_cast<char*>(&terminado), sizeof(char));
						out.write(reinterpret_cast<char*>(&cant_registros), sizeof(long int));
						typedef map<string, int>::const_iterator MapIterator;
						for (MapIterator x = indices.begin(); x != indices.end(); x++) {
							//cout << x->first << ": " << x->second << '\n';
							if(campos[1]->type == 1){
								int key = atoi((x->first).c_str());
								int offset = x->second;
								out.write(reinterpret_cast<char*>(&key), sizeof(int));
								out.write(reinterpret_cast<char*>(&offset), sizeof(int));
							}else{
								if(campos[1]->size == 1){
									char key = (x->first)[0];
									char offset = x->second;
									out.write(reinterpret_cast<char*>(&key), sizeof(char));
									out.write(reinterpret_cast<char*>(&offset), sizeof(int));
								}else{
									char* key = new char[campos[1]->size];
									strcpy(key,(x->first).c_str());
									char offset = x->second;
									out.write(key, sizeof(char)*campos[1]->size);
									out.write(reinterpret_cast<char*>(&offset), sizeof(int));
								}
							}
						}
						out.seekg(0,ios_base::beg);
						terminado = '1';
						out.write(reinterpret_cast<char*>(&terminado), sizeof(char));
						out.close();
					}	
					ofstream out(nombre_archivo, ios::out|ios::binary|ios::in);
					out.seekp(posicion-1,ios_base::beg);
					char terminado = '0';
					out.write(reinterpret_cast<char*>(&terminado),sizeof(char));
					out.write(reinterpret_cast<char*>(&avail_list),sizeof(int));
					avail_list = pos;
					out.close();
					fstream out2(nombre_archivo, ios::out|ios::binary|ios::in);
					out2.seekp(sizeof(int), ios_base::beg);

					out2.write(reinterpret_cast<char*>(&avail_list), sizeof(int));//avail list
					out2.write(reinterpret_cast<char*>(&cant_registros), sizeof(long int));
					out2.close();				

				}

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
					char borrado;
					in.read(reinterpret_cast<char*>(&borrado), sizeof(char));
					//cout<<asterisco<<endl;
					in.seekp(-sizeof(char),ios_base::cur);
					if(borrado == '0'){
						//registros_temp--;
						//cout<<"*"<<endl;
						for(int i = 0; i<campos.size(); i++){
							if(campos[i]->type == 1){
								in.seekp(sizeof(int), ios_base::cur);
							}else{
								in.seekp(sizeof(char)*campos[i]->size, ios_base::cur);
							}
						}
						j--;
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
			stringstream ss2;
			ss2<<ingresado<<".in";
			char* nombre_archivo2 = new char(ingresado.size() + 4);
			strcpy(nombre_archivo2, ss2.str().c_str());
			nombre_archivo2[ingresado.size()+3] = '\0';
			map<string,int> indices;
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
				string key;
				cout<<"Ingrese la llave de registro que desea modificar: ";
				cin>>key;
				int tam_registro = 0;
				for(int i = 0; i<campos.size(); i++){
					if(campos[i]->type == 1){
						tam_registro+= sizeof(int);
					}else{
						tam_registro+= campos[i]->size;
					}
				}
				char borrado;
				int posicion = -1;
				if(!index){
					for(int i = 0; i<cant_registros; i++){
						in.read(reinterpret_cast<char*>(&borrado), sizeof(char));
						if(borrado == '0'){
							in.seekp(tam_registro, ios_base::cur);
						}else{
							if(campos[1]->type == 1){
								int dato;
								in.read(reinterpret_cast<char*>(&dato), sizeof(int));
								stringstream ss;
								ss<<dato;
								if(ss.str() == key){
									posicion = in.tellg();
									posicion -= sizeof(int);
									break;
								}
							}else{
								if(campos[1]->size == 1){
									char dato;
									in.read(reinterpret_cast<char*>(&dato), sizeof(char));
									stringstream ss;
									ss<<dato;
									if(ss.str() == key){
										posicion = in.tellg();
										posicion -= sizeof(char);
										break;
									}
								}else{
									char* dato;
									in.read(dato,sizeof(char)*campos[1]->size);
									string ss = string(dato);
									if(dato == key){
										posicion = in.tellg();
										posicion -= sizeof(char)*campos[1]->size;
										break;
									}
								}
							}
						}
						for(int i = 2; i<campos.size(); i++){
							if(campos[i]->type == 1){
								in.seekp(sizeof(int),ios_base::cur);
							}else{
								in.seekp(sizeof(char)*campos[i]->size,ios_base::cur);
							}
						}
					}
				}else{

					if (!ReadIndex(&indices,nombre_archivo2,cant_registros, campos[1]->type, campos[1]->type)){
						cout<<"El archivo de indices no esta actualizado o no se guardo correctamente."<<endl
							<<"Porfavor reindexe el archivo antes de continuar"<<endl;
					}else{
						try{
							posicion = indices.at(key);
							posicion++;
						}catch(const std::out_of_range& oor){
							posicion = -1;
						}
					}
				}
				if(posicion == -1){
					cout<<"No se encontro un registro con esa llave"<<endl;
					if(index){
						cout<<"Puede ser que el archivo de indices no esta actualizado. Intente reindexar"<<endl;
					}
				}else{
					cout<<"Campos"<<endl;
					for(int i = 1; i<cant_campos; i++){
						cout<<i<<": ["<<campos[i]->name<<"] "<<endl;
					}
					cout<<endl;
					int modificar;
					cout<<"Ingrese la posicion del campo que desea modificar: ";
					cin>>modificar;
					for(int i = 1; i<modificar; i++){
						if(campos[i]->type == 2)
							posicion+= campos[i]->size;
						else
							posicion+= sizeof(int);
					}
					fstream out(nombre_archivo,ios_base::out|ios_base::binary|ios_base::in);
					out.seekp(posicion,ios::beg);
					if(modificar == 1 && index){
						int temp = indices.at(key);
						indices.erase(key);
						string llave;
						if(campos[modificar]->type == 1){
							int dato;
							cout<<campos[modificar]->name<<": ";
							cin>>dato;
							stringstream ss;
							ss<<dato;
							llave = ss.str();
							out.write(reinterpret_cast<char*>(&dato), sizeof(int));
						}else{
							if(campos[modificar]->size == 1){
								char dato;
								cout<<campos[modificar]->name<<": ";
								cin>>dato;
								stringstream ss;
								ss<<dato;
								llave = ss.str();
								out.write(reinterpret_cast<char*>(&dato), sizeof(char));
							}else{
								char* dato = new char[campos[modificar]->size];
								cout<<campos[modificar]->name<<": ";
								cin>>dato;
								llave = string(dato);
								out.write(reinterpret_cast<char*>(&dato), sizeof(char)*campos[modificar]->size);
							}
						}
						indices.insert(pair<string,int>(llave,temp));

					}else{
						if(campos[modificar]->type == 1){
							int dato;
							cout<<campos[modificar]->name<<": ";
							cin>>dato;
							out.write(reinterpret_cast<char*>(&dato), sizeof(int));
						}else{
							if(campos[modificar]->size == 1){
								char dato;
								cout<<campos[modificar]->name<<": ";
								cin>>dato;
								out.write(reinterpret_cast<char*>(&dato), sizeof(char));
							}else{
								char* dato = new char[campos[modificar]->size];
								cout<<campos[modificar]->name<<": ";
								cin>>dato;
								out.write(reinterpret_cast<char*>(&dato), sizeof(char)*campos[modificar]->size);
							}
						}
					}

				}
				in.close();
				if(index){
					fstream out(nombre_archivo2, ios_base::out|ios_base::binary);
					char terminado = '0';
					out.write(reinterpret_cast<char*>(&terminado), sizeof(char));
					out.write(reinterpret_cast<char*>(&cant_registros), sizeof(long int));
					typedef map<string, int>::const_iterator MapIterator;
					for (MapIterator x = indices.begin(); x != indices.end(); x++) {
						//cout << x->first << ": " << x->second << '\n';
						if(campos[1]->type == 1){
							int key = atoi((x->first).c_str());
							int offset = x->second;
							out.write(reinterpret_cast<char*>(&key), sizeof(int));
							out.write(reinterpret_cast<char*>(&offset), sizeof(int));
						}else{
							if(campos[1]->size == 1){
								char key = (x->first)[0];
								char offset = x->second;
								out.write(reinterpret_cast<char*>(&key), sizeof(char));
								out.write(reinterpret_cast<char*>(&offset), sizeof(int));
							}else{
								char* key = new char[campos[1]->size];
								strcpy(key,(x->first).c_str());
								char offset = x->second;
								out.write(key, sizeof(char)*campos[1]->size);
								out.write(reinterpret_cast<char*>(&offset), sizeof(int));
							}
						}
					}
					out.seekg(0,ios_base::beg);
					terminado = '1';
					out.write(reinterpret_cast<char*>(&terminado), sizeof(char));
					out.close();

				}


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
				Reindexar(&in,nombre_archivo2);
				in.close();
			}
		}else if (opcion == 10){

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

bool ReadIndex(map<string,int>* indices,char* name,long int cant_registros, int key_type, int key_size){
	ifstream in (name,ios_base::in|ios_base::binary);
	if(!in.is_open()){
		return false;
	}else{
		char terminado;
		in.read(reinterpret_cast<char*>(&terminado),sizeof(char));
		long int registros_map;
		in.read(reinterpret_cast<char*>(&registros_map),sizeof(long int));
		//cout<<"registros: "<<cant_registros<<endl;
		//cout<<"mapa: "<<registros_map<<endl;
		if(terminado == '0'||cant_registros != registros_map){
			return false;
		}
		for(int i = 0; i<registros_map; i++){
			if(key_type == 1){
				int key ;
				int offset;    			
				in.read(reinterpret_cast<char*>(&key), sizeof(int));
				in.read(reinterpret_cast<char*>(&offset), sizeof(int));
				stringstream ss;
				ss<<key;
				indices->insert(pair<string,int>(ss.str(),offset));
			}else{
				if(key_size == 1){
					char key;
					char offset;
					in.read(reinterpret_cast<char*>(&key), sizeof(char));
					in.read(reinterpret_cast<char*>(&offset), sizeof(int));
					stringstream ss;
					ss<<key;
					indices->insert(pair<string,int>(ss.str(),offset));
				}else{
					//char* key = new char[key_size];
					char* key;
					char offset;
					in.read(key, sizeof(char)*key_size);
					in.read(reinterpret_cast<char*>(&offset), sizeof(int));
					indices->insert(pair<string,int>(key,offset));
				}
			}
		}
	}
	in.close();
	return true;
}

void Reindexar(fstream* in,char* nombre_archivo2){
	int cant_campos;
	int avail_list;
	long int cant_registros;
	vector <Data*> campos;	
	map<string,int> indices;
	in->read(reinterpret_cast<char*>(&cant_campos),sizeof(int));
	in->read(reinterpret_cast<char*>(&avail_list),sizeof(int));
	in->read(reinterpret_cast<char*>(&cant_registros),sizeof(long int));
	campos = ReadHeader(in,cant_campos);
	fstream out(nombre_archivo2, ios_base::out|ios_base::binary);
	char terminado = '0';
	out.write(reinterpret_cast<char*>(&terminado), sizeof(char));
	out.write(reinterpret_cast<char*>(&cant_registros), sizeof(long int));
	for(int j = 0; j<cant_registros; j++){
		char borrado;
		in->read(reinterpret_cast<char*>(&borrado), sizeof(char));
		//in->seekg(-sizeof(char),ios_base::cur);
		if(borrado == '0'){
			for(int i = 1; i<campos.size(); i++){
				if(campos[i]->type == 1){
					in->seekp(sizeof(int), ios_base::cur);
				}else{
					in->seekp(sizeof(char)*campos[i]->size, ios_base::cur);
				}
			}
			j--;
		}else{

			//for(int i = 1; i<campos.size(); i++){

			if(campos[1]->type == 1){
				int dato;
				int offset = (in->tellg());
				offset--;
				in->read(reinterpret_cast<char*>(&dato), sizeof(int));
				stringstream ss;
				ss<<dato;
				string key = ss.str();
				indices.insert ( pair<string,int>(key,offset) );
			}else{
				if(campos[1]->size == 1){
					char dato;
					int offset = (in->tellg());
					offset--;
					in->read(&dato,sizeof(char));
					stringstream ss;
					ss<<dato;
					string key = ss.str();
					indices.insert ( pair<string,int>(key,offset) );
				}else{
					char* dato = new char[campos[1]->size];
					int offset = (in->tellg());
					offset--;
					in->read(dato, campos[1]->size*sizeof(char));
					string key = string(dato);
					indices.insert ( pair<string,int>(key,offset) );
				}
			}
			for(int i = 2; i<campos.size(); i++){
				if(campos[i]->type == 1){
					in->seekp(sizeof(int), ios_base::cur);
				}else{
					in->seekp(sizeof(char)*campos[i]->size, ios_base::cur);
				}

			}
			//}
		}
	}

	typedef map<string, int>::const_iterator MapIterator;
	for (MapIterator x = indices.begin(); x != indices.end(); x++) {
		//cout << x->first << ": " << x->second << '\n';
		if(campos[1]->type == 1){
			int key = atoi((x->first).c_str());
			int offset = x->second;
			out.write(reinterpret_cast<char*>(&key), sizeof(int));
			out.write(reinterpret_cast<char*>(&offset), sizeof(int));
		}else{
			if(campos[1]->size == 1){
				char key = (x->first)[0];
				char offset = x->second;
				out.write(reinterpret_cast<char*>(&key), sizeof(char));
				out.write(reinterpret_cast<char*>(&offset), sizeof(int));
			}else{
				char* key = new char[campos[1]->size];
				strcpy(key,(x->first).c_str());
				char offset = x->second;
				out.write(key, sizeof(char)*campos[1]->size);
				out.write(reinterpret_cast<char*>(&offset), sizeof(int));
			}
		}
	}
	out.seekg(0,ios_base::beg);
	terminado = '1';
	out.write(reinterpret_cast<char*>(&terminado), sizeof(char));
	out.close();
}

bool Repetido(char* nombre_archivo,string llave,long int cant_registros,vector<Data*> campos){
	
	ifstream in(nombre_archivo, ios_base::in|ios_base::binary);
	int offset = sizeof(int)*2 +sizeof(long int)+campos.size()*(sizeof(int)*2+15);
	in.seekg(offset,ios_base::beg);
	for(int i = 0; i<cant_registros; i++){
		char borrado;
		in.read(reinterpret_cast<char*>(&borrado), sizeof(char));

		if(borrado == '0'){
			//cout<<"borrado"<<endl;
			for(int j = 1; j<campos.size(); j++){
				if(campos[j]->type == 1){
					in.seekg(sizeof(int), ios_base::cur);
				}else{
					in.seekg(sizeof(char)*campos[j]->size, ios_base::cur);
				}
			}
			i--;
		}else{
			if(campos[1]->type == 1){
				int dato;
				in.read(reinterpret_cast<char*>(&dato), sizeof(int));
				//cout<<dato<<endl;
				stringstream ss;
				ss<<dato;
				if(llave == ss.str())
					return true;
			}else{
				if(campos[1]->size == 1){
					char dato;
					in.read(reinterpret_cast<char*>(&dato), sizeof(char));
					stringstream ss;
					ss<<dato;
					if(llave == ss.str())
						return true;
				}else{
					char* dato;
					in.read(dato,campos[1]->size*sizeof(char));
					string ss = string(dato);
					if(ss == llave)
						return true;
				}
			}
			for(int j = 2; j<campos.size(); j++){
				if(campos[j]->type == 1){
					in.seekg(sizeof(int), ios_base::cur);
				}else{
					in.seekg(sizeof(char)*campos[j]->size, ios_base::cur);
				}
			}
		}
	}
	//cout<<"no repetido"<<endl;
	return false;
}

