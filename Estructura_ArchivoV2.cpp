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
#include <list>
using namespace std;

int tree_order = 16;
int list_total = 0;

struct Data{
	string name;
	int type;
	int size;
};

struct Key_Offset{
	string key;
	int offset;
};

struct Node{
	int cant_key;
	int pos_padre;
	list<Key_Offset> llaves;
	list<int> hijos;
};

vector<Data*> ReadHeader(fstream* , int);
bool ReadIndex(map<string,int>* ,char* ,long int , int , int);
void Reindexar(fstream*, char*);
bool Repetido(char* ,string,long int ,vector<Data*> );
bool Busqueda_Arbol(string ,char* , int* , int , int );
bool Search(string, Node* , int*,char* ,int , int);
Node* ReadNode(fstream*, int , int ,int );
bool Insertar(char* , int , int ,int , Key_Offset , int );
void EscribirNodo(fstream* , int , int , Node*);
void CreateTree(char*,string ,int , int , int , vector<Data*> );
void Listar(Node*, int , int, char* ,char* , int ,vector<Data*>);

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
		if(opcion == '1'){				
			stringstream ss;
			cout<<"Ingrese el nombre del archivo: ";
			getline(cin,ingresado);
			if(iniciado)
				getline(cin,ingresado);
			ss<<ingresado<<".bin";
			nombre_archivo = new char(ingresado.size() + 5);
			strcpy(nombre_archivo, ss.str().c_str());
			fstream in(nombre_archivo, ios::binary|ios::in);
			campos.clear();
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
			if(index == '3'){
				CreateTree(nombre_archivo, ingresado,cant_campos, cant_registros, avail_list, campos);
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
					}else if( index == '3'){
						Key_Offset llave;
						llave.key = dato2;
						llave.offset = offset;
						stringstream ss3;
						ss3<<ingresado<<".btree";
						char* nombre_archivo3 = new char[ss3.str().size()];
						strcpy(nombre_archivo3,ss3.str().c_str());
						int pos_insertar;
						Busqueda_Arbol(llave.key,nombre_archivo3, &pos_insertar, campos[1]->type, campos[1]->size);
						Insertar(nombre_archivo3, campos[1]->type, campos[1]->size,pos_insertar, llave, -1);
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
					}else if( index == '3'){
						Key_Offset llave;
						llave.key = dato2;
						llave.offset = offset;
						stringstream ss3;
						ss3<<ingresado<<".btree";
						char* nombre_archivo3 = new char[ss3.str().size()];
						strcpy(nombre_archivo3,ss3.str().c_str());
						int pos_insertar;
						Busqueda_Arbol(llave.key,nombre_archivo3, &pos_insertar, campos[1]->type, campos[1]->size);
						Insertar(nombre_archivo3, campos[1]->type, campos[1]->size,pos_insertar, llave, -1);
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
				if(campos[1]->type == 1){
					for (MapIterator x = indices.begin(); x != indices.end(); x++) {
						indices1.insert(pair<int,int>(atoi((x->first).c_str()),x->second));

					}
				}
				typedef map<int, int>::const_iterator MapIterator2;
				int cont = 1;
				if(campos[1]->type == 1){
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
				}else{
					for (MapIterator x = indices.begin(); x != indices.end(); x++) {
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

				
			}else if(index == '3'){
				//listar
				stringstream ss3;
				ss3<<ingresado<<".btree";
				char* nombre_archivo3 = new char(ss3.str().size());
				strcpy(nombre_archivo3, ss3.str().c_str());
				fstream in(nombre_archivo3,ios::in|ios::binary);
				int root;
				in.read(reinterpret_cast<char*>(&root), sizeof(int));
				in.seekg(root,ios::beg);
				Node* nodo = ReadNode(&in, campos[1]->type, campos[1]->size, tree_order);
				in.close();
				//cout<<"keys"<<nodo->cant_key<<endl;
				//cout<<"padre"<<nodo->pos_padre<<endl;
				Listar(nodo, campos[1]->type, campos[1]->size, nombre_archivo3,nombre_archivo,sizeint, campos);
				//cout<<"total"<<list_total<<endl;
				list_total = 0;
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
			}else if (index == '3'){
				stringstream ss3;
				ss3<<ingresado<<".btree";
				char* nombre_archivo3 = new char[ss3.str().size()];
				strcpy(nombre_archivo3,ss3.str().c_str());
				if(!Busqueda_Arbol(key,nombre_archivo3, &posicion, campos[1]->type, campos[1]->size))
					posicion = -1;
				else
					posicion++;
				//cout<<"pos"<< posicion<<endl;			
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
			}else if(index == '3'){
				stringstream ss3;
				ss3<<ingresado<<".btree";
				char* nombre_archivo3 = new char[ss3.str().size()];
				strcpy(nombre_archivo3,ss3.str().c_str());
				if(!Busqueda_Arbol(key,nombre_archivo3, &posicion, campos[1]->type, campos[1]->size))
					posicion = -1;
				//cout<<"pos"<< posicion<<endl;
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
				if(index == '3'){
					CreateTree(nombre_archivo, ingresado,cant_campos, cant_registros, avail_list, campos);
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
			if(index == '3'){
				CreateTree(nombre_archivo, ingresado,cant_campos, cant_registros, avail_list, campos);
			}
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
			if(index == '3'){
				CreateTree(nombre_archivo, ingresado,cant_campos, cant_registros, avail_list, campos);
			}
			

		}else if(opcion == '8'){
			
			string ingresado2;
			cout<<"Ingrese el nombre del archivo con que lo quiere cruzar: ";
			getline(cin,ingresado2);
			getline(cin,ingresado2);
			stringstream ss2;
			ss2<<ingresado2<<".bin";
			char* nombre_archivo2 = new char[ingresado2.size()+5];
			strcpy(nombre_archivo2,ss2.str().c_str());
			fstream in1(nombre_archivo,ios::in|ios::binary);
			in1.seekg(sizeof(int)*2+sizeof(long int)+cant_campos*(15+2*sizeof(int)),ios::beg);
			fstream in2(nombre_archivo2,ios::in|ios::binary);
			if(!in2.is_open()){
				cout<<"El archivo 2 no existe"<<endl;
			}else{
				vector<Data*> campos2;
				vector<Data*> campos3;
				campos3.push_back(campos[0]);
				int cant_campos2;
				int avail_list2;
				long int cant_registros2;
				in2.read(reinterpret_cast<char*>(&cant_campos2),sizeof(int));
				in2.read(reinterpret_cast<char*>(&avail_list2),sizeof(int));
				in2.read(reinterpret_cast<char*>(&cant_registros2),sizeof(long int));
				campos2 = ReadHeader(&in2,cant_campos2);
				int inicio_registros = sizeof(int)*2+sizeof(long int)+cant_campos*(15+2*sizeof(int));

				string resultado;
				cout<<"Ingrese el nombre del archivo donde quiere guardar el cruce: ";
				getline(cin,resultado);
				char* nombre_archivo3 = new char[resultado.size()+5];
				stringstream resultado2;
				resultado2<<resultado<<".bin";
				strcpy(nombre_archivo3,resultado2.str().c_str());
				fstream out(nombre_archivo3, ios::out|ios::binary);
				cout<<"Campos archivo 1"<<endl;
				for(int i = 1; i<campos.size(); i++){
					cout<<i<<" :"<<campos[i]->name<<endl;
				}
				int pos_campo1;
				cout<<"Ingrese el numero que campo que quiere cruzar: ";
				cin>>pos_campo1;
				int offset_campo1 = 0;
				for(int k = 0; k<pos_campo1; k++){
					if(campos[k]->type == 1)
						offset_campo1 += sizeof(int);
					else
						offset_campo1 += campos[k]->size;
				}
				//campos3.push_back(campos[pos_campo1]);
				int campo_cruzar1;
				vector<int> campos_cruzar1;

				do{
					cout<<"Ingrese el numero de campo que quiere incluir en el cruce(-1 para dejar de ingresar): ";
					cin>>campo_cruzar1;
					if(campo_cruzar1 != -1){
						campos3.push_back(campos[campo_cruzar1]);
						int offset_temp = 0;
						for(int k = 0; k<campo_cruzar1; k++){
							if(campos[k]->type == 1)
								offset_temp += sizeof(int);
							else
								offset_temp += campos[k]->size;
						}
						campos_cruzar1.push_back(offset_temp);
					}
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
				int offset_campo2 = 0;
				for(int k = 0; k<pos_campo2; k++){
					if(campos2[k]->type == 1)
						offset_campo2 += sizeof(int);
					else
						offset_campo2 += campos2[k]->size;
				}
				int campo_cruzar2;
				vector<int> campos_cruzar2;
				do{
					cout<<"Ingrese el numero de campo que quiere incluir en el cruce(-1 para dejar de ingresar): ";
					cin>>campo_cruzar2;
					if(campo_cruzar2 != -1){
						campos3.push_back(campos2[campo_cruzar2]);
						int offset_temp = 0;
						for(int k = 0; k<campo_cruzar2; k++){
							if(campos2[k]->type == 1)
								offset_temp += sizeof(int);
							else
								offset_temp += campos2[k]->size;
						}
						campos_cruzar2.push_back(offset_temp);
					}
					//campos_cruzar2.push_back(campo_cruzar2);
				}while(campo_cruzar2 != -1);

				int temp_availist = 0;
				long int cant_registros3 = 0;
				int campos3_temp = campos3.size();
				out.write(reinterpret_cast<char*>(&campos3_temp),sizeof(int));
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
					else{
						tam_registro2+= campos2[i]->size;
						//cout<<"campos "<<i<<campos2[i]->size<<endl;
					}
				}
				if(index == '1'){
					//cout<<"Cruzar sin indices"<<endl;
					/*for(int i = 0; i<campos3.size(); i++){
						cout<<campos3[i]->name<<endl;
					}*/
					for(int i =0; i<cant_registros; i++){
						string llave_compartida;
						in1.seekg(offset_campo1,ios::cur);
						if(campos[pos_campo1]->type == 1){
							int dato;
							in1.read(reinterpret_cast<char*>(&dato),sizeof(int));
							in1.seekg(-sizeof(int),ios::cur);
							stringstream ss3;
							ss3<<dato;
							llave_compartida = ss3.str();
						}else if(campos[pos_campo1]->size == 1){
							char dato;
							in1.read(reinterpret_cast<char*>(&dato),sizeof(char));
							in1.seekg(-sizeof(char),ios::cur);
							stringstream ss3;
							ss3<<dato;
							llave_compartida = ss3.str();
						}else{
							char* dato = new char[campos[pos_campo1]->size];
							in1.read(dato,sizeof(char)*campos[pos_campo1]->size);
							in1.seekg(-sizeof(char)*campos[pos_campo1]->size,ios::cur);
							llave_compartida = string(dato);
						}
						in2.seekg(inicio_registros,ios::beg);
						in1.seekg(-offset_campo1,ios::cur);
						for(int j = 0; j<cant_registros2; j++){
							string llave_compartida2;
							//cout<<"offset "<<offset_campo2<<endl;
							//cout<<"actual "<<in2.tellg()<<endl;

							in2.seekg(offset_campo2,ios::cur);
							if(campos2[pos_campo2]->type == 1){
								int dato;
								in2.read(reinterpret_cast<char*>(&dato),sizeof(int));
								in2.seekg(-sizeof(int),ios::cur);
								stringstream ss3;
								ss3<<dato;
								llave_compartida2 = ss3.str();
							}else if(campos2[pos_campo2]->size == 1){
								char dato;
								in2.read(reinterpret_cast<char*>(&dato),sizeof(char));
								in2.seekg(-sizeof(char),ios::cur);
								stringstream ss3;
								ss3<<dato;
								llave_compartida2 = ss3.str();
							}else{
								char* dato = new char[campos2[pos_campo2]->size];
								in2.read(dato,sizeof(char)*campos2[pos_campo2]->size);
								in2.seekg(-sizeof(char)*campos2[pos_campo2]->size,ios::cur);
								llave_compartida2 = string(dato);
							}
							//cout<<llave_compartida<<" "<<llave_compartida2<<endl;
							in2.seekg(-offset_campo2,ios::cur);
							if(llave_compartida == llave_compartida2){
								//cout<<"llave igual"<<endl;
								//in1.seekg(,ios::cur);
								cant_registros3++;
								char borrado = '1';
								out.write(reinterpret_cast<char*>(&borrado),sizeof(char));
								//in.seekg(sizeof(char),ios::cur);
								int acumulador = 1;
								for(int k = 0; k<campos_cruzar1.size(); k++){
									in1.seekg(campos_cruzar1[k],ios::cur);
									if(campos3[acumulador]->type == 1){
										int dato;
										in1.read(reinterpret_cast<char*>(&dato),sizeof(int));
										//cout<<dato<<endl;
										out.write(reinterpret_cast<char*>(&dato),sizeof(int));
										in1.seekg(-sizeof(int),ios::cur);
									}else if(campos3[acumulador]->size == 1){
										char dato;
										in1.read(reinterpret_cast<char*>(&dato),sizeof(char));
										//cout<<dato<<endl;
										out.write(reinterpret_cast<char*>(&dato),sizeof(char));
										in1.seekg(-sizeof(char),ios::cur);
									}else{
										char* dato = new char[campos3[acumulador]->size];
										in1.read(dato,sizeof(char)*campos3[acumulador]->size);
										//cout<<dato<<endl;
										for(int x = 0; x<campos3[acumulador]->size; x++)
											out.write(reinterpret_cast<char*>(&(dato[x])),sizeof(char));
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
										//cout<<dato<<endl;
										out.write(reinterpret_cast<char*>(&dato),sizeof(int));
										in2.seekg(-sizeof(int),ios::cur);
									}else if(campos3[acumulador]->size == 1){
										char dato;
										in2.read(reinterpret_cast<char*>(&dato),sizeof(char));
										//cout<<dato<<endl;
										out.write(reinterpret_cast<char*>(&dato),sizeof(char));
										in2.seekg(-sizeof(char),ios::cur);
									}else{
										char* dato = new char[campos3[acumulador]->size];
										in2.read(dato,sizeof(char)*campos3[acumulador]->size);
										//cout<<dato<<endl;
										for(int x = 0; x<campos3[acumulador]->size; x++)
											out.write(reinterpret_cast<char*>(&(dato[x])),sizeof(char));
										in2.seekg(-campos3[acumulador]->size,ios::cur);
									}
									acumulador++;
									in2.seekg(-campos_cruzar2[k],ios::cur);
								}
							}else{

							}
							in2.seekg(tam_registro2,ios::cur);
						}
						in1.seekg(tam_registro1,ios::cur);
						
					} 
					
					
				}else if(index == '2'){
					map<string,int> indices2;
					char* nombre_archivo4 = new char[ingresado2.size()+4];
					stringstream ss4;
					ss4<<ingresado2<<".in";
					strcpy(nombre_archivo4,ss4.str().c_str());
					fstream in_temp(nombre_archivo2, ios::in|ios::binary);
					Reindexar(&in_temp,nombre_archivo4);
					in_temp.close();
					ReadIndex(&indices2,nombre_archivo4,cant_registros2, campos2[1]->type, campos2[1]->size);
					typedef map<string, int>::const_iterator MapIterator;
					MapIterator x1 = indices.begin();
					MapIterator x2 = indices2.begin();
					while(x1 != indices.end() && x2 != indices2.end()){
						if(x1->first == x2->first){
							in1.seekg(x1->second,ios::beg);
							in2.seekg(x2->second,ios::beg);
							cant_registros3++;
							char borrado = '1';
							out.write(reinterpret_cast<char*>(&borrado),sizeof(char));
								//in.seekg(sizeof(char),ios::cur);
							int acumulador = 1;
							for(int k = 0; k<campos_cruzar1.size(); k++){
								in1.seekg(campos_cruzar1[k],ios::cur);
								if(campos3[acumulador]->type == 1){
									int dato;
									in1.read(reinterpret_cast<char*>(&dato),sizeof(int));
										//cout<<dato<<endl;
									out.write(reinterpret_cast<char*>(&dato),sizeof(int));
									in1.seekg(-sizeof(int),ios::cur);
								}else if(campos3[acumulador]->size == 1){
									char dato;
									in1.read(reinterpret_cast<char*>(&dato),sizeof(char));
										//cout<<dato<<endl;
									out.write(reinterpret_cast<char*>(&dato),sizeof(char));
									in1.seekg(-sizeof(char),ios::cur);
								}else{
									char* dato = new char[campos3[acumulador]->size];
									in1.read(dato,sizeof(char)*campos3[acumulador]->size);
										//cout<<dato<<endl;
									for(int x = 0; x<campos3[acumulador]->size; x++)
										out.write(reinterpret_cast<char*>(&(dato[x])),sizeof(char));
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
										//cout<<dato<<endl;
									out.write(reinterpret_cast<char*>(&dato),sizeof(int));
									in2.seekg(-sizeof(int),ios::cur);
								}else if(campos3[acumulador]->size == 1){
									char dato;
									in2.read(reinterpret_cast<char*>(&dato),sizeof(char));
										//cout<<dato<<endl;
									out.write(reinterpret_cast<char*>(&dato),sizeof(char));
									in2.seekg(-sizeof(char),ios::cur);
								}else{
									char* dato = new char[campos3[acumulador]->size];
									in2.read(dato,sizeof(char)*campos3[acumulador]->size);
										//cout<<dato<<endl;
									for(int x = 0; x<campos3[acumulador]->size; x++)
										out.write(reinterpret_cast<char*>(&(dato[x])),sizeof(char));
									in2.seekg(-campos3[acumulador]->size,ios::cur);
								}
								acumulador++;
								in2.seekg(-campos_cruzar2[k],ios::cur);
							}
							x1++;
							x2++;
						}else if(x1->first<x2->first){
							x1++;
						}else{
							x2++;
						}						
					}

				}else{

				}
				in1.close();
				in2.close();
				out.close();
				fstream out2(nombre_archivo3, ios::out|ios::binary|ios::in);
				out2.seekp(sizeof(int)*2,ios::beg);
				out2.write(reinterpret_cast<char*>(&cant_registros3),sizeof(long int));
				out2.close();
			}
		}else if(opcion == '9'){

		}else{
			cout<<"La opcion no es valida"<<endl;
		}
		iniciado = true;
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


/*bool Busqueda_Arbol(string buscado,char* nombre_archivo2,int* pos_buscado, int type_key, int size_key){
	fstream in(nombre_archivo2,ios::in|ios::binary);
	int pos_root;
	int order = tree_order;

	//int cant_keys;
	//in.read(reinterpret_cast<char*>(&root), sizeof(int));
	//in.read(reinterpret_cast<char*>(&cant_keys), sizeof(int));
	in.read(reinterpret_cast<char*>(&pos_root), sizeof(int));
	//cout<<"pos root "<<pos_root<<endl;
	int posnodo_insertar = pos_root;
	in.seekp(pos_root,ios::beg);
	Node* actual = ReadNode(&in,type_key,size_key,order);
	//cout<<"lee nodo"<<endl;
	//cout<<"tamaño"<<actual->cant_key<<endl;
	int nodo_siguiente = -1;
	do{	
		nodo_siguiente = -1;
		int cont = 0;
		bool menor = true;
		for (list<Key_Offset>::iterator it=(actual->llaves).begin(); it != (actual->llaves).end(); ++it){
			cout<<it->key<<endl;
			cout<<buscado<<endl;
			if(it->key == buscado){
				//cout<<it->offset<<endl;
				*pos_buscado = it->offset;
				in.close();
				return true;
			}else if(buscado < it->key){
				menor = false;
				list<int>::iterator it2 = (actual->hijos).begin();
				for(int i = 0; i<cont; i++ ){
					++it2;
				}
				nodo_siguiente = *it2;
				//if(nodo_siguiente != -1)
				//	in.seekp(nodo_siguiente,ios::beg);
				break;
			}
			if(buscado > it->key && it->key == actual->llaves.back().key && menor){
				nodo_siguiente = actual->hijos.back();
			}
			cont++;
		}
    	//if(nodo_siguiente == -1 && menor){
    		//list<int>::iterator it2 = (actual->hijos).end();
    		//--it2;
    		//cout<<"ultimo"<<*it2<<endl;
		//	nodo_siguiente = actual->hijos.back();
			//nodo_siguiente = *it2;
    	//}
    	//cout<<"siguente "<<nodo_siguiente<<endl;
    	if(nodo_siguiente != -1){
    		//cout<<"diferente"<<endl;
			in.seekp(nodo_siguiente,ios::beg);
			posnodo_insertar = in.tellp();
			actual = ReadNode(&in,type_key,size_key,order);
    	}
	}while(nodo_siguiente != -1);

	*pos_buscado = posnodo_insertar;
	//cout<<"salio"<<endl;
	//cout<<"pos buscado"<<*pos_buscado<<endl;
	in.close();
	return false;
}*/
bool Busqueda_Arbol(string buscado,char* nombre_archivo2,int* posnodo_insertar, int type_key, int size_key){
	fstream in(nombre_archivo2,ios::in|ios::binary);
	int pos_root;
	int order = tree_order;
	in.read(reinterpret_cast<char*>(&pos_root), sizeof(int));
	//cout<<"pos root "<<pos_root<<endl;
	*posnodo_insertar = pos_root;
	in.seekp(pos_root,ios::beg);
	Node* actual = ReadNode(&in,type_key,size_key,order);
	in.close();
	return Search(buscado, actual, posnodo_insertar,nombre_archivo2,type_key, size_key);
}

bool Search(string buscado, Node* root, int* pos,char* nombre_archivo,int type_key, int size_key){
	list<int>::iterator it2 = root->hijos.begin();

	for (list<Key_Offset>::iterator it=(root->llaves).begin(); it != (root->llaves).end(); ++it){
		//cout<<it->key<<endl;
		if(it->key == buscado){
			*pos = it->offset;
			return true;
		}else{
			if(type_key == 1){
				if (atoi(buscado.c_str()) < atoi(it->key.c_str())){
					break;
				}
			}else{
				if (buscado < it->key){
					break;
				}
			}
		}
		it2++;

	}
	bool retorno = false;
	if(*it2 != -1){
		fstream in(nombre_archivo,ios::in|ios::binary);
		in.seekg(*it2,ios::beg);
		Node* neo = ReadNode(&in,type_key,size_key,tree_order);
		*pos = *it2;
		retorno  = retorno || Search(buscado,neo,pos,nombre_archivo,type_key,size_key);
	}
	return retorno;
}

Node* ReadNode(fstream* in, int type_key, int size_key,int order){
	Node* nodo = new Node;
	Key_Offset* key_offset = new Key_Offset;
	int cant_keys;
	in->read(reinterpret_cast<char*>(&cant_keys), sizeof(int));
	//cout<<"cant_key "<<cant_keys<<endl;
	nodo->cant_key = cant_keys;
	int offset_padre;
	in->read(reinterpret_cast<char*>(&offset_padre), sizeof(int));
	//cout<<"offset padre "<<offset_padre<<endl;
	nodo->pos_padre = offset_padre;
	int offset;
	in->read(reinterpret_cast<char*>(&offset), sizeof(int));
	//cout<<"offset "<<offset<<endl;
	nodo->hijos.push_back(offset);
	for(int i =0; i<cant_keys; i++){
		key_offset = new Key_Offset;
		if(type_key == 1){
			int dato;
			in->read(reinterpret_cast<char*>(&dato), sizeof(int));
			//cout<<"dato "<<dato<<endl;
			stringstream ss;
			ss<<dato;
			key_offset->key = ss.str();
			int dato2;
			in->read(reinterpret_cast<char*>(&dato2), sizeof(int));
			key_offset->offset = dato2;
			nodo->llaves.push_back(*key_offset);
		}else if(size_key == 1){
			char dato;
			in->read(reinterpret_cast<char*>(&dato), sizeof(char));
			stringstream ss;
			ss<<dato;
			key_offset->key = ss.str();
			int dato2;
			in->read(reinterpret_cast<char*>(&dato2), sizeof(int));
			key_offset->offset = dato2;
			nodo->llaves.push_back(*key_offset);
		}else{
			char* dato = new char[size_key];
			in->read(dato, sizeof(char)*size_key);
			key_offset->key = string(dato);
			int dato2;
			in->read(reinterpret_cast<char*>(&dato2), sizeof(int));
			key_offset->offset = dato2;
			nodo->llaves.push_back(*key_offset);
		}
		in->read(reinterpret_cast<char*>(&offset), sizeof(int));
		//cout<<"offset "<<offset<<endl;
		nodo->hijos.push_back(offset);
	}
	/*for(int i = cant_keys; i<order-1; i++){
		if(type_key == 1)
			in.seekp(sizeof(int),ios::cur);
		else
			in.seekp(size_key,ios::cur);
		in.seekp(sizeof(int),ios::cur);
	}*/
	return nodo;
}

bool Insertar(char* nombre_archivo, int key_type, int size_key,int pos_insertar, Key_Offset llave, int rrn_split){
	fstream in(nombre_archivo,ios::binary|ios::in);
	in.seekp(pos_insertar,ios::beg);
	Node* nodo = ReadNode(&in,key_type,size_key,tree_order);
	in.close();
	list<Key_Offset>::iterator it = nodo->llaves.begin();
	list<int>::iterator it2 = nodo->hijos.begin();
	//cout<<"hijo "<<*it2<<endl;
	++it2;
	if(key_type == 1){
		while(it !=nodo->llaves.end()){//for (it=(nodo->llaves).begin(); it != (nodo->llaves).end(); ++it){
			//cout<<"llave"<<it->key<<endl;
			if(atoi((it->key).c_str()) > atoi(llave.key.c_str())){
				break;
			}
			//cout<<"hijo "<<*it2<<endl;
			++it;
			++it2;		
		}
	}else{
		while(it !=nodo->llaves.end()){//for (it=(nodo->llaves).begin(); it != (nodo->llaves).end(); ++it){
			//cout<<"llave"<<it->key<<endl;
			if(it->key > llave.key){
				break;
			}
			//cout<<"hijo "<<*it2<<endl;
			++it;
			++it2;		
		}
	}
		
	//cout<<"llave final"<<(*it).key<<endl;
	//cout<<"hijo final"<<*it2<<endl;
	nodo->llaves.insert(it,llave);
	nodo->hijos.insert(it2, rrn_split);
	//cout<<"tam nuevo "<<nodo->llaves.size()<<endl;
	nodo->cant_key = nodo->llaves.size();
	if(nodo->llaves.size() == tree_order){
		list<Key_Offset>::iterator it3 = nodo->llaves.begin();
		//list<int>::iterator it4 = nodo->hijos.begin();
		//++it4;
		for(int i = 0; i<nodo->llaves.size()/2-1; i++){
			++it3;
			//++it4;
		}
		fstream in(nombre_archivo,ios::binary|ios::in);
		in.seekp(0,ios::end);
		int temp = in.tellp();
		in.close();
		//split
		Node* split1 = new Node;
		Node* split2 = new Node;
		split1->cant_key = nodo->llaves.size()/2-1;
		split1->pos_padre = nodo->pos_padre;
		split2->cant_key = nodo->llaves.size()/2;
		split2->pos_padre = nodo->pos_padre;
		list<Key_Offset>::iterator it4 = nodo->llaves.begin();
		list<int>::iterator it5 = nodo->hijos.begin();
		split1->hijos.push_back(*it5);
		++it5;
		for(int i = 0; i<nodo->llaves.size(); i++){
			if(i == nodo->llaves.size()/2-1){
				split2->hijos.push_back(*it5);
			}else if(i < nodo->llaves.size()/2-1){
				split1->llaves.push_back(*it4);
				split1->hijos.push_back(*it5);
			}else{
				split2->llaves.push_back(*it4);
				split2->hijos.push_back(*it5);
			}
			++it4;
			++it5;
		}
		//split2->hijos.push_back(*it4);
		
		if(nodo->pos_padre != -1){
			//sobrescribir en posicion nodo con mitad
			fstream out(nombre_archivo,ios::out|ios::binary|ios::in);
			out.seekg(pos_insertar,ios::beg);
			EscribirNodo(&out,key_type,size_key,split1);
			//escribir otra mitad al final
			out.seekg(temp,ios::beg);
			EscribirNodo(&out,key_type,size_key,split2);
			out.close();
			Insertar(nombre_archivo,key_type,size_key,nodo->pos_padre,*it3,temp);
		}else{
			int size_pagina = sizeof(int)*2 + tree_order*sizeof(int);
			if(key_type == 1){
				size_pagina += (tree_order-1)*(sizeof(int)+sizeof(int));
			}else{
				size_pagina += (tree_order-1)*(sizeof(int)+size_key);
			}
			split1->pos_padre = temp + size_pagina;
			split2->pos_padre = temp + size_pagina;		

			//sobrescribir en posicion nodo con mitad
			fstream out(nombre_archivo,ios::out|ios::binary|ios::in);
			out.seekg(pos_insertar,ios::beg);
			EscribirNodo(&out,key_type,size_key,split1);
			//escribir otra mitad al final
			out.seekg(temp,ios::beg);
			EscribirNodo(&out,key_type,size_key,split2);
			
			Node* neoroot = new Node;
			neoroot->cant_key = 1;
			neoroot->pos_padre = -1;
			
			neoroot->llaves.push_back(*it3);
			neoroot->hijos.push_back(pos_insertar);
			neoroot->hijos.push_back(temp);
			EscribirNodo(&out,key_type,size_key,neoroot);
			out.close();
			fstream out2(nombre_archivo,ios::in|ios::out|ios::binary);
			int newroot = temp+size_pagina;			
			out2.write(reinterpret_cast<char*>(&newroot), sizeof(int));
			out2.close();
			//Insertar(nombre_archivo,key_type,size_key,pos_padre,*it3,temp);
		}
		return true;
	}else{
		//sobrescribir nuevo
		fstream out(nombre_archivo,ios::out|ios::binary|ios::in);
		out.seekg(pos_insertar,ios::beg);
		//cout<<"tamaño"<<nodo->cant_key<<endl;
		EscribirNodo(&out,key_type,size_key,nodo);
		out.close();
		return false;
	}
}

void EscribirNodo(fstream* out, int key_type, int key_size, Node* nodo){
	int cant_keys = nodo->cant_key;
	int pos_padre = nodo->pos_padre;
	out->write(reinterpret_cast<char*>(&cant_keys), sizeof(int));
	out->write(reinterpret_cast<char*>(&pos_padre), sizeof(int));
	list<int>::iterator it1 = nodo->hijos.begin();
	out->write(reinterpret_cast<char*>(&(*it1)), sizeof(int));
	for(list<Key_Offset>::iterator it2 = nodo->llaves.begin();it2 != nodo->llaves.end(); ++it2){
		if(key_type == 1){
			int dato;
			dato = atoi(it2->key.c_str());
			out->write(reinterpret_cast<char*>(&dato), sizeof(int));
			int offset = it2->offset;
			out->write(reinterpret_cast<char*>(&offset), sizeof(int));
		}else if(key_size == 1){
			char dato;
			dato = it2->key[0];
			out->write(reinterpret_cast<char*>(&dato), sizeof(char));
			int offset = it2->offset;
			out->write(reinterpret_cast<char*>(&offset), sizeof(int));
		}else{
			char* dato = new char[key_size];
			strcpy(dato,it2->key.c_str());
			out->write(dato, sizeof(char)*key_size);
			int offset = it2->offset;
			out->write(reinterpret_cast<char*>(&offset), sizeof(int));
		}
		++it1;
		out->write(reinterpret_cast<char*>(&(*it1)), sizeof(int));
		
	}
	int espacio = -1;
	for(int i = cant_keys; i<(tree_order-1); i++){
		if(key_type == 1){
			out->write(reinterpret_cast<char*>(&espacio), sizeof(int));
			out->write(reinterpret_cast<char*>(&espacio), sizeof(int));
		}else if(key_size==1) {
			char espacio2 = '0';
			out->write(reinterpret_cast<char*>(&espacio2), sizeof(char));
			out->write(reinterpret_cast<char*>(&espacio), sizeof(int));
		}else{
			char* espacio2 = new char[key_size];
			out->write(espacio2, sizeof(char)*key_size);
			out->write(reinterpret_cast<char*>(&espacio), sizeof(int));
		}
		out->write(reinterpret_cast<char*>(&espacio), sizeof(int));
	}

}

void CreateTree(char* nombre_archivo, string ingresado,int cant_campos, int cant_registros, int avail_list, vector<Data*> campos){
	stringstream ss;
	ss<<ingresado<<".btree";
	char* nombre_archivo2 = new char[ss.str().size()];
	strcpy(nombre_archivo2,ss.str().c_str());
	fstream prueba(nombre_archivo2,ios::in|ios::binary);
	if(prueba.is_open()){
		prueba.close();
		remove(nombre_archivo2);
	}else
		prueba.close();
	fstream in(nombre_archivo,ios::in|ios::binary);	
	int size_header = sizeof(int)*2+sizeof(long int)+cant_campos*(sizeof(int)*2+15);
	in.seekg(size_header,ios::beg);
	for(int j = 0; j<cant_registros; j++){
		char borrado;
		in.read(reinterpret_cast<char*>(&borrado), sizeof(char));
		//in->seekg(-sizeof(char),ios_base::cur);
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
			if(campos[1]->type == 1){
				int dato;
				int offset = (in.tellg());
				offset--;
				in.read(reinterpret_cast<char*>(&dato), sizeof(int));
				stringstream ss;
				ss<<dato;
				string key = ss.str();
				if(j == 0){
					fstream out(nombre_archivo2,ios::out|ios::binary);
					int root = sizeof(int);
					int cant_keys = 1;
					int pos_padre = -1;
					int hijo = -1;
					out.write(reinterpret_cast<char*>(&root), sizeof(int));
					out.write(reinterpret_cast<char*>(&cant_keys), sizeof(int));
					out.write(reinterpret_cast<char*>(&pos_padre), sizeof(int));
					out.write(reinterpret_cast<char*>(&hijo), sizeof(int));
					out.write(reinterpret_cast<char*>(&dato), sizeof(int));
					out.write(reinterpret_cast<char*>(&offset), sizeof(int));
					out.write(reinterpret_cast<char*>(&hijo), sizeof(int));
					out.close();
				}else{
					Key_Offset llave;
					llave.key = key;
					llave.offset = offset;
					int pos_insertar;
					Busqueda_Arbol(llave.key,nombre_archivo2, &pos_insertar, campos[1]->type, campos[1]->size);
					//cout<<"pos "<<pos_insertar<<endl;
					Insertar(nombre_archivo2,campos[1]->type,campos[1]->size,pos_insertar,llave,-1);
				}
			}else{
				if(campos[1]->size == 1){
					char dato;
					int offset = (in.tellg());
					offset--;
					in.read(&dato,sizeof(char));
					stringstream ss;
					ss<<dato;
					string key = ss.str();

					if(j == 0){
						fstream out(nombre_archivo2,ios::out|ios::binary);
						int root = 0;
						int cant_keys = 1;
						int pos_padre = -1;
						int hijo = -1;
						out.write(reinterpret_cast<char*>(&root), sizeof(int));
						out.write(reinterpret_cast<char*>(&cant_keys), sizeof(int));
						out.write(reinterpret_cast<char*>(&pos_padre), sizeof(int));
						out.write(reinterpret_cast<char*>(&hijo), sizeof(int));
						out.write(reinterpret_cast<char*>(&dato), sizeof(char));
						out.write(reinterpret_cast<char*>(&offset), sizeof(int));
						out.write(reinterpret_cast<char*>(&hijo), sizeof(int));
						out.close();
					}else{
						Key_Offset llave;
						llave.key = key;
						llave.offset = offset;
						int pos_insertar;
						//cout<<pos_insertar<<endl;
						Busqueda_Arbol(llave.key,nombre_archivo2, &pos_insertar, campos[1]->type, campos[1]->size);
						Insertar(nombre_archivo2,campos[1]->type,campos[1]->size,pos_insertar,llave,-1);
					}
				}else{
					char* dato = new char[campos[1]->size];
					int offset = (in.tellg());
					offset--;
					in.read(dato, campos[1]->size*sizeof(char));
					string key = string(dato);

					if(j == 0){
						fstream out(nombre_archivo2,ios::out|ios::binary);
						int root = 0;
						int cant_keys = 1;
						int pos_padre = -1;
						int hijo = -1;
						out.write(reinterpret_cast<char*>(&root), sizeof(int));
						out.write(reinterpret_cast<char*>(&cant_keys), sizeof(int));
						out.write(reinterpret_cast<char*>(&pos_padre), sizeof(int));
						out.write(reinterpret_cast<char*>(&hijo), sizeof(int));
						out.write(dato, sizeof(char)*campos[1]->size);
						out.write(reinterpret_cast<char*>(&offset), sizeof(int));
						out.write(reinterpret_cast<char*>(&hijo), sizeof(int));
						out.close();
					}else{
						Key_Offset llave;
						llave.key = key;
						llave.offset = offset;
						int pos_insertar;
						Busqueda_Arbol(llave.key,nombre_archivo2, &pos_insertar, campos[1]->type, campos[1]->size);
						//cout<<pos_insertar<<endl;
						Insertar(nombre_archivo2,campos[1]->type,campos[1]->size,pos_insertar,llave,-1);
					}
				}
			}
			for(int i = 2; i<campos.size(); i++){
				if(campos[i]->type == 1){
					in.seekp(sizeof(int), ios_base::cur);
				}else{
					in.seekp(sizeof(char)*campos[i]->size, ios_base::cur);
				}

			}
			//}
		}
	}
	in.close();
}

void Listar(Node* nodo, int type_key, int size_key, char* nombre_archivo,char* nombre_archivo2, int sizeint,vector<Data*> campos){
	list<int>::iterator it = nodo->hijos.begin();
	if(*it != -1){
		fstream in(nombre_archivo,ios::in|ios::binary);
		in.seekg(*it,ios::beg);
		Node* hijo = ReadNode(&in,type_key,size_key,tree_order);
		in.close();
		Listar(hijo,type_key,size_key,nombre_archivo,nombre_archivo2,sizeint,campos);	
	}
	++it;
	//int j = 0;
	for(list<Key_Offset>::iterator it2 = nodo->llaves.begin(); it2 != nodo->llaves.end(); ++it2){
		//listar con offset
		fstream in2(nombre_archivo2,ios::in|ios::binary);
		in2.seekg(it2->offset,ios::beg);
		char borrado;
		in2.read(reinterpret_cast<char*>(&borrado), sizeof(char));
		if(borrado == '0'){
			/*for(int i = 1; i<campos.size(); i++){
				if(campos[i]->type == 1){
					in.seekp(sizeof(int), ios_base::cur);
				}else{
					in.seekp(sizeof(char)*campos[i]->size, ios_base::cur);
				}
			}
			j--;*/
		}else{
			list_total++;
			cout<<"Registro "<<setw(10)<<left;
			for(int i = 1; i<campos.size(); i++){
				if(campos[i]->type == 1){
					int dato;
					in2.read(reinterpret_cast<char*>(&dato), sizeof(int));
									//cout<<campos[i]->name<<": "<<dato<<endl;
					cout<<setw(sizeint)<<right<<dato;
				}else{
					if(campos[i]->size == 1){
						char dato;
						in2.read(&dato,sizeof(char));
										//cout<<campos[i]->name<<": "<<dato<<endl;
						cout<<setw(campos[i]->size+15)<<right<<dato;
					}else{
						char* dato = new char[campos[i]->size];
						in2.read(dato, campos[i]->size*sizeof(char));

										//cout<<campos[i]->name<<": "<<dato<<endl;
						cout<<setw(campos[i]->size+15)<<right<<dato;
					}
				}

			}
			cout<<endl;
		}
		/*if(j%10 == 0 && j!=0){
			char respuesta2;
			cout<<"Ingrese 1 para ver 10 registros mas: ";
			cin>>respuesta2;
			if(respuesta2 != '1')
				break;
		}*/

		if(*it != -1){
			fstream in(nombre_archivo,ios::in|ios::binary);
			in.seekg(*it,ios::beg);
			Node* hijo = ReadNode(&in,type_key,size_key,tree_order);
			in.close();
			Listar(hijo,type_key,size_key,nombre_archivo,nombre_archivo2,sizeint, campos);	
		}
		++it;
		//j++;	
		in2.close();	
	}
}