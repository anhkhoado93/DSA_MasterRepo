#ifndef VM_H
#define VM_H

#include "main.h"

typedef int address;


/*Declarations*/
enum type {INT, FLOAT, BOOL, ADDR, NONE};
struct datatype;
class VM;
class CPU;
class RStack;
template<typename T>
class List;

/*Datatype*/
struct datatype
{
    type typ = type::NONE;
    union {
      address dvalue;
      int ivalue;
      float fvalue;
      bool bvalue;
    };
    // T value;
    int addr = -1;
    datatype* next = nullptr;
    datatype(){}
    
    void updateType(type t,  float val)
    {
      typ = t;
      ivalue = 0;

      switch(t)
      {
      case INT: 
        ivalue = (int)val;
        break;
      case FLOAT: 
        fvalue = val;
        break;
      case BOOL:
        bvalue = (bool)val;
        break;
      case ADDR: 
        dvalue = (int)val;
        break; 
      case NONE:
          ivalue = 0;
          break;  
      }
    }
    void updateType(datatype& d)
    {
      typ = d.typ;
      switch(typ)
      {
      case INT: 
        ivalue = d.ivalue;
        break;
      case FLOAT: 
        fvalue = d.fvalue;
        break;
      case BOOL:
        bvalue = d.bvalue;
        break;
      case ADDR: 
        dvalue = d.dvalue;
        break; 
      case NONE:
        ivalue = 0;
        break; 
      }
    }
    ~datatype(){}

};

struct istring
{
    string s_arr[3] = {"", "", ""};
    int addr = -1;
    int n = 0;
    istring* next = nullptr;

    istring(string s_arr[3], int a, int n)
    {
      for (int i = 0; i < 3; ++i)
        this->s_arr[i] = s_arr[i];
      this->addr = a;
      this->n = n;
    }
};

class CPU
{
  datatype registers[16];
  int ins_ptr = 0;
  List<istring>* prog;
  datatype* sref;
  RStack* stack;
public:
  CPU(){}
  CPU(List<istring>*, datatype*, RStack*);
  void execute();
  type getType(string&str);
  datatype& loadMem(int addr);
  datatype& loadReg(string& str);
  void clean();
};

template<typename T>
class List 
{
  T* head;
  T*  tail;
  int size;
public:
  List();
  int   append(T&);
  int   insertFront(T&);
  int   getSize();
  T*    pop_back();
  T*    pop_front();
  T*    atIndex(int);
  int   getMaxIndex();
  T*    operator[](int);
  T*    begin();
  void  clear();
};


class RStack
{
  List<datatype>* stack;
public:
  RStack();
  ~RStack();
  void      push(datatype&);
  datatype* pop();
  int       getStackSize();
  void      updateReg(int, type, string);
  bool      isEmpty();
  void      clear();
};


class VM
{
  List<istring>* mem_bank;
  datatype* smem_bank;
  CPU* cpu;
  //List<datatype> cache;
  RStack* ret_addr;
public:
  VM()
  {
    mem_bank 	= new List<istring>;
    smem_bank 	= new datatype[2<<16];
    ret_addr = new RStack();
    cpu 		= new CPU(mem_bank, smem_bank, ret_addr);
  }
  ~VM()
  {
    clean();
  }
  void run(string filename);
  void loadProgram(string);
  void checkSyntax(string[],int , int);
  void clean();
};



class Util
{
public:
  static bool is_in(string[], int, string&);
  static bool isReg(string&);
  static bool isDec(string&);
  static bool isFlt(string&);
  static bool isBoo(string&);
  static bool isAdr(string&);
  static bool isLit(string&);
  static bool isSrc(string&);
};
#endif