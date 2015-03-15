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
	char opcion = '1';
	char index = '1';
	string ingresado;
	char* nombre_archivo;
	vector<Data*> campos;
	int cant_campos;
	int avail_list;
	long int cant_registros;
	map<string,int> indices;
	bool iniciado = false;
	do{
		if(iniciado){
			cout<<"Menu"<<endl
				<<"1)Abrir Archivo"<<endl
				<<"2)Insertar Datos"<<endl
				<<"3)Listar"<<endl
				<<"4)Buscar"<<endl
				<<"5)Borrar"<<endl
				<<"6)Compactar"<<endl
				<<"7)Modificar"<<endl
				<<"8)Cruzar Archivos"<<endl
				<<"9)Salir"<<endl
				<<"Ingrese su opcion: ";
			cin>>opcion;
		}
		iniciado = true;
		if(opcion == '1'){				
			stringstream ss;
			cout<<"Ingrese el nombre del archivo: ";
			getline(cin,ingresado);
			ss<<ingresado<<".bin";
			nombre_archivo = new char(ingresado.size() + 5);
			strcpy(nombre_archivo, ss.str().c_str());
			fstream in(nombre_archivo, ios::binary|ios::in);
			if(in.is_open()){
				in.read(reinterpret_cast<char*>(&cant_campos),sizeof(int));
				in.read(reinterpret_cast<char*>(&avail_list),sizeof(int));
				in.read(reinterpret_cast<char*>(&cant_registros),sizeof(long int));
				campos = ReadHeader(&in,cant_campos);
				in.close();
			}else{
				in.close();
				fstream out(nombre_archivo, ios::binary|ios::out);
				int continuar;
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
				//escribir header
				cant_campos = campos.size();
				avail_list = 0;
				cant_registros = 0;
				out.write(reinterpret_cast<char*>(&cant_campos), sizeof(int));//numero de campos
				out.write(reinterpret_cast<char*>(&avail_list), sizeof(int));//avail list
				out.write(reinterpret_cast<char*>(&cant_registros), sizeof(long int));//espacio para cantidad de registros
				for(int i = 0; i<campos.size(); i++){
					for(int j =0 ; j<15; j++){
						out.write(reinterpret_cast<char*>(&((campos[i]->name)[j])), sizeof(char));
					}
					out.write(reinterpret_cast<char*>(&(campos[i]->type)), sizeof(int));
					out.write(reinterpret_cast<char*>(&(campos[i]->size)), sizeof(int));
				}
				out.close();
			}
			do{			
				cout<<"Indices"<<endl
					<<"1)No usar indices"<<endl
					<<"2)Usar indices lineales"<<endl
					<<"3)Usar Arbol B"<<endl
					<<"Ingrese su opcion: ";
				cin>>index;
			}while(index!='1' && index!='2' && index!='3');

			if(index == '2'){
				stringstream ss2;
				ss2<<ingresado<<".in";
				char* nombre_archivo2 = new char(ingresado.size() + 4);
				strcpy(nombre_archivo2, ss2.str().c_str());
				nombre_archivo2[ingresado.size()+3] = '\0';
				fstream in(nombre_archivo, ios::in|ios::binary);
				Reindexar(&in,nombre_archivo2);
				in.close();
				ReadIndex(&indices,nombre_archivo2,cant_registros, campos[1]->type, campos[1]->size);
			}
		}else if(opcion == '2'){
			stringstream ss2;
			ss2<<ingresado<<".in";
			char* nombre_archivo2 = new char(ingresado.size() + 4);
			strcpy(nombre_archivo2, ss2.str().c_str());
			nombre_archivo2[ingresado.size()+3] = '\0';
			/*fstream in(nombre_archivo, ios::in|ios::binary);
			in.read(reinterpret_cast<char*>(&cant_campos),sizeof(int));
			in.read(reinterpret_cast<char*>(&avail_list),sizeof(int));
			in.read(reinterpret_cast<char*>(&cant_registros),sizeof(long int));
			campos = ReadHeader(&in,cant_campos);
			in.close();*/
			int disponible = avail_list;
			string dato2;
			int offset;
			string llave_ingresada;		

			if(avail_list ==0){
				ofstream out(nombre_archivo, ios::out|ios::binary|ios::app);
				int continuar;
				out.seekp(0,ios::end);
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
					if(index == '2'){
						indices.insert(pair<string,int>(dato2,offset));
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

					if(index == '2'){
						indices.insert(pair<string,int>(dato2,offset));						
					}

					out.close();
				}while(continuar == 1 && avail_list != 0);	
			}
			fstream out2(nombre_archivo, ios::out|ios::binary|ios::in);
			out2.seekp(sizeof(int), ios_base::beg);
			out2.write(reinterpret_cast<char*>(&avail_list), sizeof(int));//nuevo avail_list
			out2.write(reinterpret_cast<char*>(&cant_registros), sizeof(long int));//cantidad de registros
			out2.close();
			if(index == '2'){
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
		}else if(opcion == '3'){
			fstream in(nombre_archivo, ios::in|ios::binary);
			int sizeint = 10;
			in.seekg(sizeof(int)*2+sizeof(long int)+cant_campos*(15+sizeof(int)*2),ios::beg);
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
			if(index == '1'){
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
			}else if(index == '2'){
				map<int,int> indices1;
				stringstream ss2;
				ss2<<ingresado<<".in";
				char* nombre_archivo2 = new char(ingresado.size() + 4);
				strcpy(nombre_archivo2, ss2.str().c_str());
				nombre_archivo2[ingresado.size()+3] = '\0';
				typedef map<string, int>::const_iterator MapIterator;
				for (MapIterator x = indices.begin(); x != indices.end(); x++) {
					indices1.insert(pair<int,int>(atoi((x->first).c_str()),x->second));

				}
				typedef map<int, int>::const_iterator MapIterator2;
				int cont = 1;
				for (MapIterator2 x = indices1.begin(); x != indices1.end(); x++) {
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
			in.close();	
			
		}else if(opcion == '4'){
			stringstream ss2;
			ss2<<ingresado<<".in";
			char* nombre_archivo2 = new char(ingresado.size() + 4);
			strcpy(nombre_archivo2, ss2.str().c_str());
			nombre_archivo2[ingresado.size()+3] = '\0';
			fstream in(nombre_archivo, ios::in|ios::binary);
			in.seekg(sizeof(int)*2+sizeof(long int)+cant_campos*(15+sizeof(int)*2),ios::beg);
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
			if(index == '1'){
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
			}else if(index == '2'){
				try{
					posicion = indices.at(key);
					posicion++;
				}catch(const std::out_of_range& oor){
					posicion = -1;
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

		}else if(opcion == '5'){
			stringstream ss2;
			ss2<<ingresado<<".in";
			char* nombre_archivo2 = new char(ingresado.size() + 4);
			strcpy(nombre_archivo2, ss2.str().c_str());
			nombre_archivo2[ingresado.size()+3] = '\0';
			fstream in(nombre_archivo, ios::in|ios::binary);
			int pos = 0;
			int offset2 = sizeof(int)*2 +sizeof(long int)+cant_campos*(sizeof(int)*2+15);
			in.seekg(sizeof(int)*2+sizeof(long int)+cant_campos*(15+sizeof(int)*2),ios::beg);
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
			if(index == '1'){
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
			}else if(index == '2'){
				try{
					posicion = indices.at(key);
					posicion++;
				}catch(const std::out_of_range& oor){
					posicion = -1;
				}
			}
			if(posicion == -1){
				cout<<"No se encontro un registro con esa llave"<<endl;
			}else{
					//indices.erase(key_borrar);
				int pos = (posicion-1-offset2)/tam_registro +1;	
				cant_registros--;
				if(index == '2'){
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
		

		}else if(opcion == '6'){
			stringstream ss2;
			ss2<<ingresado<<"1.bin";
			char* nombre_archivo2 = new char(ingresado.size() + 6);
			strcpy(nombre_archivo2, ss2.str().c_str());
			fstream in(nombre_archivo, ios::in|ios::binary);
			fstream out(nombre_archivo2, ios::out|ios::binary);
			avail_list = 0;
			//cout<<"campos "<<cant_campos<<endl<<"registros "<<cant_registros<<endl;
			out.write(reinterpret_cast<char*>(&cant_campos),sizeof(int));
			out.write(reinterpret_cast<char*>(&avail_list),sizeof(int));
			out.write(reinterpret_cast<char*>(&cant_registros),sizeof(long int));
			for(int i = 0; i<cant_campos; i++){
				char* name = new char[15];
				strcpy(name,(campos[i]->name).c_str());
				name[14] = '\0';
				int type = campos[i]->type;
				int size = campos[i]->size;
				for(int j = 0; j<15*sizeof(char); j++){
					out.write(reinterpret_cast<char*>(&(name[j])),sizeof(char));
				}
				out.write(reinterpret_cast<char*>(&type),sizeof(int));
				out.write(reinterpret_cast<char*>(&size),sizeof(int));
			}
			in.seekg(sizeof(int)*2 +sizeof(long int)+cant_campos*(sizeof(int)*2+15),ios::beg);
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
								in.read(reinterpret_cast<char*>(&dato), sizeof(char));
								out.write(reinterpret_cast<char*>(&dato),sizeof(char));
							}else{
								char* dato = new char[campos[i]->size];
								in.read(dato, campos[i]->size*sizeof(char));
								dato[(campos[i]->size)-1] = '\0';
								for(int k = 0; k<(campos[i]->size)*sizeof(char); k++){
									out.write(reinterpret_cast<char*>(&(dato[k])),sizeof(char));
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

			//fstream out2(nombre_archivo, ios::out|ios::binary|ios::in);
			//out2.seekp(sizeof(int), ios_base::beg);
				//cout<<"registros "<<cant_registros<<endl;
			//int temp_avail = 0;
			//out2.write(reinterpret_cast<char*>(&temp_avail), sizeof(int));//avail list
			//out2.write(reinterpret_cast<char*>(&cant_registros), sizeof(long int));//cant registros
			//out2.close();
		}else if(opcion == '7'){
			stringstream ss2;
			ss2<<ingresado<<".in";
			char* nombre_archivo2 = new char(ingresado.size() + 4);
			strcpy(nombre_archivo2, ss2.str().c_str());
			nombre_archivo2[ingresado.size()+3] = '\0';
			fstream in(nombre_archivo, ios::in|ios::binary);
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
			in.seekg(sizeof(int)*2 +sizeof(long int)+cant_campos*(sizeof(int)*2+15),ios::beg);
			if(index == '1'){
				for(int i = 0; i<cant_registros; i++){
					in.read(reinterpret_cast<char*>(&borrado), sizeof(char));
					if(borrado == '0'){
						in.seekp(tam_registro-1, ios_base::cur);
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
			}else if(index == '2'){
				try{
					posicion = indices.at(key);
					posicion++;
				}catch(const std::out_of_range& oor){
					posicion = -1;
				}
			}

			if(posicion == -1){
				cout<<"No se encontro un registro con esa llave"<<endl;
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
				if(modificar == 1 && index == '2'){
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
							for(int i = 0; i<campos[modificar]->size; i++){
								out.write(reinterpret_cast<char*>(&dato[i]), sizeof(char));
							}
								//out.write(reinterpret_cast<char*>(&dato), sizeof(char)*campos[modificar]->size);
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
							for(int i = 0; i<campos[modificar]->size; i++){
								out.write(reinterpret_cast<char*>(&dato[i]), sizeof(char));
							}
								//out.write(reinterpret_cast<char*>(&dato), sizeof(char)*campos[modificar]->size);
						}
					}
				}

			}
			in.close();
			if(index == '2'){
				fstream out(nombre_archivo2, ios_base::out|ios_base::binary);
				char terminado = '0';
				out.write(reinterpret_cast<char*>(&terminado), sizeof(char));
				out.write(reinterpret_cast<char*>(&cant_registros), sizeof(long int));
				typedef map<string, int>::const_iterator MapIterator;
				for (MapIterator x = indices.begin(); x != indices.end(); x++) {
						//cout << x->first << ": " << x->second << '\n';
					if(campos[1]->type == 1){
						int key2 = atoi((x->first).c_str());
						int offset = x->second;
						out.write(reinterpret_cast<char*>(&key2), sizeof(int));
						out.write(reinterpret_cast<char*>(&offset), sizeof(int));
					}else{
						if(campos[1]->size == 1){
							char key2 = (x->first)[0];
							char offset = x->second;
							out.write(reinterpret_cast<char*>(&key2), sizeof(char));
							out.write(reinterpret_cast<char*>(&offset), sizeof(int));
						}else{
							char* key2 = new char[campos[1]->size];
							strcpy(key2,(x->first).c_str());
							char offset = x->second;
							out.write(key2, sizeof(char)*campos[1]->size);
							out.write(reinterpret_cast<char*>(&offset), sizeof(int));
						}
					}
				}
				out.seekg(0,ios_base::beg);
				terminado = '1';
				out.write(reinterpret_cast<char*>(&terminado), sizeof(char));
				out.close();

			}
			

		}else if(opcion == '8'){
			
			string ingresado2;
			cout<<"Ingrese el nombre del archivo con que lo quiere cruzar: ";
			getline(cin,ingresado2);
			stringstream ss2;
			ss2<<ingresado2<<".bin";
			char* nombre_archivo2 = new char[15];
			strcpy(nombre_archivo2,ss2.str().c_str());
			ifstream in1(nombre_archivo,ios::in|ios::bin);
			in1.seekg(sizeof(int)*2+sizeof(long int)+cant_campos*(15+2*sizeof(int)),ios::beg);
			ifstream in2(nombre_archivo2,ios::in|ios::bin);
			if(!in2.is_open()){
				cout<<"El archivo 2 no existe"<<endl:
			}else{
				vector<Data*> campos2;
				vector<Data*> campos3;
				campos3.push_back(campos2[0]);
				int cant_campos2;
				int avail_list2;
				long int cant_registros2;
				in2.read(reinterpret_cast<char*>(&cant_campos2),sizeof(int));
				in2.read(reinterpret_cast<char*>(&avail_list2),sizeof(int));
				in2.read(reinterpret_cast<char*>(&cant_registros2),sizeof(long int));
				campos = ReadHeader(&in2,cant_campos2);
				char* nombre_archivo3 = new char[15];
				cout<<"Ingrese el nombre del archivo donde quiere guardar el cruce: ";
				cin>>nombre_archivo3;
				nombre_archivo3[14] = '\0';
				fstream out(nombre_archivo3, ios::out|ios::bin);
				cout<<"Campos archivo 1"<<endl;
				for(int i = 1; i<campos.size(); i++){
					cout<<i<<" :"<<campos[i]->name<<endl;
				}
				int pos_campo1;
				cout<<"Ingrese el numero que campo que quiere cruzar: ";
				cin>>pos_campo1;

				campos3.push_back(campos[pos_campo1]);
				int campo_cruzar1;
				vector<int> campos_cruzar1;

				do{
					cout<<"Ingrese el numero de campo que quiere incluir en el cruce(-1 para dejar de ingresar): ";
					cin>>campo_cruzar1;
					campos3.push_back(campos[campo_cruzar1]);
					int offset_temp = 0;
					for(int k = 0; k<campo_cruzar1; k++){
						if(campos[k]->type == 1)
							offset_temp += sizeof(int);
						else
							offset_temp += campos[k]->size;
					}
					campo_cruzar1.push_back(offset_temp);
					//campos_cruzar1.push_back(campo_cruzar1);
				}while(campo_cruzar1 != -1);

				cout<<"Campos archivo 2"<<endl;
				for(int i = 1; i<campos2.size(); i++){
					cout<<i<<" :"<<campos2[i]->name<<endl;
				}
				int pos_campo2;
				bool validado;
				do{
					cout<<"Ingrese el numero que campo que quiere cruzar: ";
					cin>>pos_campo2;
					if(campos[pos_campo1]->type != campos2[pos_campo2]->type){
						validado = false;
						cout<<"El tipo de los campos no concuerda"<<endl;
					}else
						validado = true;
				}while(!validado);
				int campo_cruzar2;
				vector<int> campos_cruzar2;
				do{
					cout<<"Ingrese el numero de campo que quiere incluir en el cruce(-1 para dejar de ingresar): ";
					cin>>campo_cruzar2;
					campos3.push_back(campos[campo_cruzar2]);
					int offset_temp = 0;
					for(int k = 0; k<campo_cruzar2; k++){
						if(campos2[k]->type == 1)
							offset_temp += sizeof(int);
						else
							offset_temp += campos2[k]->size;
					}
					campo_cruzar2.push_back(offset_temp);
					//campos_cruzar2.push_back(campo_cruzar2);
				}while(campo_cruzar2 != -1);

				int temp_availist = 0;
				long int cant_registros3 = 0;
				out.write(reinterpret_cast<char*>(&(campos3.size())),sizeof(int));
				out.write(reinterpret_cast<char*>(&temp_availist),sizeof(int));
				out.write(reinterpret_cast<char*>(&cant_registros3),sizeof(long int));
				for(int i = 0; i<campos3.size(); i++){
					for(int j =0 ; j<15; j++){
						out.write(reinterpret_cast<char*>(&((campos3[i]->name)[j])), sizeof(char));
					}
					out.write(reinterpret_cast<char*>(&(campos3[i]->type)), sizeof(int));
					out.write(reinterpret_cast<char*>(&(campos3[i]->size)), sizeof(int));
				}
				int tam_registro1 = 0;
				for(int i = 0; i<campos.size(); i++){
					if(campos[i]->type == 1)
						tam_registro1 += sizeof(int);
					else
						tam_registro1+= campos[i]->size;
				}
				int tam_registro2 = 0;
				for(int i = 0; i<campos2.size(); i++){
					if(campos2[i]->type == 1)
						tam_registro2 += sizeof(int);
					else
						tam_registro2+= campos2[i]->size;
				}
				if(index == '1'){
					for(int i =0; i<cant_registros; i++){
						string llave_compartida;
						in1.seekg(sizeof(char),ios::cur);
						if(campos3[1]->type == 1){
							int dato;
							in1.read(reinterpret_cast<char*>(&dato),sizeof(int));
							in1.seekg(-sizeof(int)-1,ios::cur);
							stringstream ss3;
							ss3<<dato;
							llave_compartida = ss3.str();
						}else if(campos3[1]->size == 1){
							char dato;
							in1.read(reinterpret_cast<char*>(&dato),sizeof(char));
							in1.seekg(-sizeof(char)-1,ios::cur);
							stringstream ss3;
							ss3<<dato;
							llave_compartida = ss3.str();
						}else{
							char* dato = new char[campos3[1]->size];
							in1.read(dato,sizeof(char)*campos3->size);
							in1.seekg(-sizeof(char)*campos3->size-1,ios::cur);
							llave_compartida = string(dato);
						}

						for(int j = 0; j<cant_registros2; j++){
							string llave_compartida2;
							in2.seekg(sizeof(char),ios::cur);
							if(campos3[1]->type == 1){
								int dato;
								in2.read(reinterpret_cast<char*>(&dato),sizeof(int));
								in2.seekg(-sizeof(int)-1,ios::cur);
								stringstream ss3;
								ss3<<dato;
								llave_compartida2 = ss3.str();
							}else if(campos3[1]->size == 1){
								char dato;
								in2.read(reinterpret_cast<char*>(&dato),sizeof(char));
								in2.seekg(-sizeof(char)-1,ios::cur);
								stringstream ss3;
								ss3<<dato;
								llave_compartida2 = ss3.str();
							}else{
								char* dato = new char[campos3[1]->size];
								in2.read(dato,sizeof(char)*campos3->size);
								in2.seekg(-sizeof(char)*campos3->size-1,ios::cur);
								llave_compartida2 = string(dato);
							}
							if(llave_compartida == llave_compartida2){
								char borrado = '1';
								out.write(reinterpret_cast<char*>(&borrado),sizeof(char));
								//in.seekg(sizeof(char),ios::cur);
								int acumulador = 1;
								for(int k = 0; k<campos_cruzar1.size(); k++){
									in1.seekg(campos_cruzar1[k],ios::cur);
									if(campos3[acumulador]->type == 1){
										int dato;
										in1.read(reinterpret_cast<char*>(&dato),sizeof(int));
										out.write(reinterpret_cast<char*>(&dato),sizeof(int));
										in1.seekg(-sizeof(int),ios::cur);
									}else if(campos3[acumulador]->size == 1){
										char dato;
										in1.read(reinterpret_cast<char*>(&dato),sizeof(char));
										out.write(reinterpret_cast<char*>(&dato),sizeof(char));
										in1.seekg(-sizeof(char),ios::cur);
									}else{
										char* dato = new char[campos3[acumulador]->size];
										in1.read(dato,sizeof(char)*campos3[acumulador]->size);
										out.write(dato,sizeof(char)*campos3->size);
										in1.seekg(-campos3[acumulador]->size,ios::cur);
									}
									acumulador++;
									in1.seekg(-campos_cruzar1[k],ios::cur);
								}
								for(int k = 0; k<campos_cruzar2.size(); k++){
									in2.seekg(campos_cruzar2[k],ios::cur);
									if(campos3[acumulador]->type == 1){
										int dato;
										in2.read(reinterpret_cast<char*>(&dato),sizeof(int));
										out.write(reinterpret_cast<char*>(&dato),sizeof(int));
										in2.seekg(-sizeof(int),ios::cur);
									}else if(campos3[acumulador]->size == 1){
										char dato;
										in2.read(reinterpret_cast<char*>(&dato),sizeof(char));
										out.write(reinterpret_cast<char*>(&dato),sizeof(char));
										in2.seekg(-sizeof(char),ios::cur);
									}else{
										char* dato = new char[campos3[acumulador]->size];
										in2.read(dato,sizeof(char)*campos3[acumulador]->size);
										out.write(dato,sizeof(char)*campos3->size);
										in2.seekg(-campos3[acumulador]->size,ios::cur);
									}
									acumulador++;
									in2.seekg(-campos_cruzar2[k],ios::cur);
								}
							}
						}
						out.close();
					} 
					in1.close();
					in2.close();
				}else if(index == '2'){

				}else{

				}
			}
		}else if(opcion == '9'){

		}else{
			cout<<"La opcion no es valida"<<endl;
		}
	}while(opcion != '9');

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