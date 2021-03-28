#include "VM.h"



bool Util::is_in(string list[], int size, string& ins)
{
	for(int i = 0; i < size; ++i)
	{
		if (ins == list[i]) return 1;
	}
	return 0;
}

bool Util::isReg(string& str)
{
	return regex_match(str, regex("R([1-9]|1[0-5])"));
}

bool Util::isDec(string& str)
{
	return regex_match(str, regex("0|([1-9][0-9]*)"));
}

bool Util::isFlt(string& str)
{
	return regex_match(str, regex("([1-9][0-9]*)(.[0-9]+)"));
}

bool Util::isBoo(string& str)
{
	return regex_match(str, regex("true|false"));
}

bool Util::isAdr(string& str)
{
	return (regex_match(str, regex("[0-9]+A")) 
		&& (str.length() < 5 || str.length() == 5 && str < "65536"));
}

bool Util::isLit(string& str)
{
	return isDec(str) || isFlt(str) || isBoo(str) || isAdr(str);
}

bool Util::isSrc(string& str)
{
	return isReg(str) || isLit(str);
}



/*========================================================================*/



void VM::run(string filename)
{
	this->loadProgram(filename);
	cpu->execute();
}

void VM::loadProgram(string file)
{
	fstream fs;
	string ins;
	fs.open(file, ios::in);
	int addr = 0;

	while (getline(fs,ins))
	{
		if (ins.back() == '\r') ins.pop_back();
		
		if(not regex_match(ins, regex("[^ ]+( [^ ]+(, [^ ]+)?)?")))
			throw InvalidInstruction(addr);
		
		regex re("\\s+");
		regex_token_iterator<string::iterator> first(ins.begin(), ins.end(), re, -1);
		regex_token_iterator<string::iterator> end, counter = first;
		string temp[3] = {"", "", ""};
		int N;
		for (N = 0; N < 3; ++N, ++first)
		{
			if (first == end) break;
			temp[N] = string(*first);
		}
		checkSyntax(temp, N, addr);
		istring* s = new istring(temp, addr, N);
		mem_bank->append(*s);

		++addr;
	}
	fs.close();
}

void VM::clean()
{
	mem_bank->clear();
	ret_addr->clear();
	cpu->clean();
	
	delete mem_bank;
	delete[] smem_bank;
	delete cpu;
	delete ret_addr;
}

void VM::checkSyntax(string s_arr[],int size , int addr)
{
	string two_op[16] = {
	"Add", "Mul", "Minus", "Div","CompNE", "CmpEQ", 
	"CmpLT", "CmpLE", "CmpGT", "CmpGE", "Add", "Or",
	"Load", "Store", "Move", "JumpIf"
	};
	string uni_op_dest[2] = {"Not", "Input"};
	string uni_op_src[3] = {"Jump", "Call", "Output"};
	string no_op[2]	= {"Return", "Halt"};

	string ins = s_arr[0];


		if(Util::is_in(two_op, 16, ins))
		{
			if (size != 3) throw InvalidInstruction(addr);
			string& dest = s_arr[1], src = s_arr[2];
			s_arr[1].pop_back();
			if (!Util::isReg(dest) || !Util::isSrc(src)) 
				throw InvalidOperand(addr);
		}
		else if (Util::is_in(uni_op_dest, 2, ins))
		{
			if (size != 2) throw InvalidInstruction(addr);
			string& dest = s_arr[1];
			if (!Util::isReg(dest)) throw InvalidOperand(addr);
		}
		else if (Util::is_in(uni_op_src, 3, ins))
		{
			if (size != 2) throw InvalidInstruction(addr);
			string& src = s_arr[1];
			if (!Util::isSrc(src)) throw InvalidOperand(addr);
		}
		else if (Util::is_in(no_op, 2, ins))
		{
			if (size != 1) throw InvalidInstruction(addr);
		}
		else throw InvalidInstruction(addr);
}



/*========================================================================*/



CPU::CPU(List<istring>* mem, datatype* smem, RStack* s)
{
	prog = mem;
	sref = smem;
	stack = s;
} 

void CPU::execute()
{
	string arithm[4] = {"Add", "Mul", "Minus", "Div"};
	string cmpare[6] = {"CmpNE", "CmpEQ", 
		"CmpLT", "CmpLE", "CmpGT", "CmpGE"};
	string logins[3] = {"Not", "And", "Or"};
	List<istring>& ptr = *prog;
	istring* ins = ptr[ins_ptr];
	while(true)
	{
		string cmd = ins->s_arr[0];
		int addr = ins->addr;
		try
		{
		if(Util::is_in(arithm, 4, cmd))
		{
			type __op1 = getType(ins->s_arr[1]);
			type __op2 = getType(ins->s_arr[2]);

			bool cond = (__op1 == type::INT ||  __op1 == type::FLOAT) && 
					(__op2 == type::INT ||  __op2 == type::FLOAT);
			if (!cond) throw TypeMismatch(addr);
			
			type __re = (__op1 == type::INT && __op2 == type::INT)? (type::INT): (type::FLOAT);

			float __o1, __o2, ans;
			datatype& reg1 = loadReg(ins->s_arr[1]);
			__o1 = (__op1 == type::INT)? (float)reg1.ivalue: reg1.fvalue;
			
			if (ins->s_arr[2][0] == 'R')
			{
				datatype& reg2 = loadReg(ins->s_arr[2]);
				__o2 = (__op2 == type::INT)? (float)reg2.ivalue: reg2.fvalue;
			}
			else 
			{
				__o2 = (__op2 == type::INT)? (float)stoi(ins->s_arr[2]): stof(ins->s_arr[2]);
			}
			if(cmd == "Add") 		ans = __o1 + __o2;
			else if(cmd == "Minus") ans = __o1 - __o2;
			else if(cmd == "Mul") 	ans = __o1 * __o2;
			else if(cmd == "Div") 	
			{
				if (__o2 == 0) throw DivideByZero(addr);
				ans = __o1 / __o2;
			}

			reg1.updateType(__re, ans);
		}
		
		else if(Util::is_in(cmpare, 4, cmd))
		{
			type __op1 = getType(ins->s_arr[1]);
			type __op2 = getType(ins->s_arr[2]);
			bool cond1 = (__op1 == type::INT ||  __op1 == type::FLOAT) && (__op2 == type::INT ||  __op2 == type::FLOAT);
			bool cond2 = __op1 == type::BOOL &&  __op2 == type::BOOL && (cmd == "CmpEQ" || cmd == "CmpNE");
			if (!(cond1 || cond2)) 
				throw TypeMismatch(addr);
			

			float __o1, __o2, ans;
			datatype& reg1 = loadReg(ins->s_arr[1]);
			__o1 = ((__op1 == type::FLOAT)? reg1.fvalue: (__op1 == type::INT)? (float)reg1.ivalue: (float)reg1.bvalue);
			
			if (ins->s_arr[2][0] == 'R')
			{
				datatype& reg2 = loadReg(ins->s_arr[2]);
				__o2 = ((__op2 == type::FLOAT)? reg2.fvalue: (__op2 == type::INT)? (float)reg2.ivalue: (float)reg2.bvalue);
			}
			else 
			{
				__o2 = ((__op2 == type::FLOAT)? stof(ins->s_arr[2]): (__op2 == type::INT)? stof(ins->s_arr[2]): (float)(ins->s_arr[2] == "true"));
			}
			if(cmd == "CmpEQ") 			ans = (__o1 == __o2);
			else if(cmd == "CmpNE") 	ans = (__o1 != __o2);
			else if(cmd == "CmpLE") 	ans = (__o1 <= __o2);
			else if(cmd == "CmpGE")		ans = (__o1 >= __o2);
			else if(cmd == "CmpGT")		ans = (__o1 >  __o2);
			else if(cmd == "CmpLT") 	ans = (__o1 <  __o2);


			reg1.updateType(type::BOOL, (bool)ans);
		}

		else if(Util::is_in(logins, 3, cmd))
		{
			datatype& reg = loadReg(ins->s_arr[1]);
			if (reg.typ != type::BOOL) throw TypeMismatch(addr);
			bool __o1 = reg.bvalue;
			if (cmd == "Not")
				reg.updateType(type::BOOL, !__o1);
			else
			{
				bool __o2;
				if(ins->s_arr[2][0] == 'R')  
				{
					datatype& ref = loadReg(ins->s_arr[2]);
					if (ref.typ != BOOL) throw TypeMismatch(addr);
					__o2 = loadReg(ins->s_arr[2]).bvalue;
				}
				else  
				{
					if (ins->s_arr[2] != "true" && ins->s_arr[2] != "false") 
						throw TypeMismatch(addr);
					__o2 = ins->s_arr[2] == "true";
				}
				if (cmd == "And") reg.updateType(type::BOOL, __o1 && __o2);
				else reg.updateType(type::BOOL, __o1 || __o2);
			}
		
		}
		else if (cmd == "Input")
		{
			string s;
			cin >> s;
			if (!Util::isLit(s) || Util::isAdr(s)) throw InvalidInput(addr);
			type typ = getType(s);
			if (s == "true" || s == "false") (s = (s == "true")? "1": "0");
			loadReg(ins->s_arr[1]).updateType(typ, stof(s));
		}
		else  if (cmd == "Output")
		{
			if (ins->s_arr[1][0] == 'R')
			{
				datatype& t = loadReg(ins->s_arr[1]);
				if (t.typ == NONE) throw TypeMismatch(addr);
				else if (t.typ == INT) cout <<  t.ivalue;
				else if (t.typ == FLOAT) cout << t.fvalue;
				else if (t.typ == BOOL) cout << ((t.bvalue)? "true":"false");
				else if (t.typ == ADDR) cout << t.dvalue << 'A';
			}
			else
			{
				if(!Util::isLit(ins->s_arr[1])) throw TypeMismatch(addr);
				cout << ins->s_arr[1];
			}
		}
		else if (cmd == "Move")
		{
			datatype& reg1 = loadReg(ins->s_arr[1]);
			if (ins->s_arr[2][0] == 'R')
			{
				datatype& reg2 = loadReg(ins->s_arr[2]);
				if (reg2.typ == type::NONE)
					throw TypeMismatch(addr);
				reg1.updateType(reg2);
			}
			else
			{
				if (!Util::isLit(ins->s_arr[2]))
					throw TypeMismatch(addr);
				type __op2 = getType(ins->s_arr[2]);
				if (__op2 == INT) reg1.updateType(__op2, stoi(ins->s_arr[2]));
				else if (__op2 == FLOAT) reg1.updateType(__op2, stof(ins->s_arr[2]));
				else if (__op2 == BOOL) reg1.updateType(__op2, ins->s_arr[2] == "true");
				else if (__op2 == ADDR) 
				{
					string temp = ins->s_arr[2];
					temp.pop_back();
					reg1.updateType(__op2, stoi(temp));
				}
			}
		}
		else if (cmd == "Load")
		{
			datatype& reg1 = loadReg(ins->s_arr[1]);
			if (getType(ins->s_arr[2]) != type::ADDR)
				throw TypeMismatch(addr);
			if (ins->s_arr[2][0] == 'R')
			{
				datatype& t = loadReg(ins->s_arr[2]);
				datatype& d = loadMem(t.dvalue);
				reg1.updateType(d);
			}
			else
			{
				string temp = ins->s_arr[2];
				temp.pop_back();
				datatype& ref = loadMem(stoi(temp));
				reg1.updateType(ref);
			}
		}
		else if (cmd == "Store")
		{
			datatype& reg1 = loadReg(ins->s_arr[1]);
			type __op2 = getType(ins->s_arr[2]);
			if (reg1.typ != type::ADDR || __op2 == type::NONE)
				throw TypeMismatch(addr);
			datatype& ref = loadMem(reg1.dvalue);
			if (ins->s_arr[2][0] == 'R')
			{
				datatype& reg2 = loadReg(ins->s_arr[2]);
				ref.updateType(reg2);
			}
			else
			{
				if (__op2 == INT) ref.updateType(__op2, stoi(ins->s_arr[2]));
				else if (__op2 == FLOAT) ref.updateType(__op2, stof(ins->s_arr[2]));
				else if (__op2 == BOOL) ref.updateType(__op2, ins->s_arr[2] == "true");
				else if (__op2 == ADDR) 
				{
					string temp = ins->s_arr[2];
					temp.pop_back();
					ref.updateType(__op2, stoi(temp));
				}
			}
			
		}
		else if (cmd == "Halt") break;
		else if (cmd == "Jump")
		{
			if (getType(ins->s_arr[1]) != type::ADDR)
				throw TypeMismatch(addr);
			int addr_v;
			if(ins->s_arr[1][0] == 'R')
			{
				datatype& ref = loadReg(ins->s_arr[1]);
				addr_v = ref.dvalue;
			}
			else
			{
				string a =ins->s_arr[1];
				a.pop_back();
				addr_v = stoi(a);
			}
			if (addr_v  < 0 || addr_v >= ptr.getSize())
				throw InvalidDestination(addr);
			ins_ptr = addr_v;
			ins = ptr[ins_ptr];
			// Skip increment 
			continue;
		}
		else if (cmd == "JumpIf")  
		{
			if (getType(ins->s_arr[1]) != type::BOOL || getType(ins->s_arr[2]) != type::ADDR)
				throw TypeMismatch(addr);
			bool s = loadReg(ins->s_arr[1]).bvalue;
			if (s)
			{
				int addr_v;
				if(ins->s_arr[2][0] == 'R')
				{
					datatype& ref = loadReg(ins->s_arr[2]);
					addr_v = ref.dvalue;
				}
				else
				{
					string a =ins->s_arr[2];
					a.pop_back();
					addr_v = stoi(a);
				}
				if (addr_v  < 0 || addr_v > ptr.getSize())
					throw InvalidDestination(addr);
				ins_ptr = addr_v;
				ins = ptr[ins_ptr];
				// Skip increment 
				continue;
			}
			
		}
		else if (cmd == "Call")
		{
			if (getType(ins->s_arr[1]) != type::ADDR)
				throw TypeMismatch(addr);
			int addr_v;
			if(ins->s_arr[1][0] == 'R')
			{
				datatype& ref = loadReg(ins->s_arr[1]);
				addr_v = ref.dvalue;
			}
			else
			{
				string a =ins->s_arr[1];
				a.pop_back();
				addr_v = stoi(a);
			}
			if (addr_v  < 0 || addr_v >= ptr.getSize())
				throw InvalidDestination(addr);
			datatype* d = new datatype();
			d->typ = ADDR;
			d->dvalue = ins_ptr;
			// push current address to stack
			if (stack->getStackSize() >= 1000) throw StackFull(addr);
			stack->push(*d);
			//
			ins_ptr = addr_v;
			ins = ptr[ins_ptr];
			// Skip increment 
			continue;
		}
		else if (cmd == "Return")
		{
			datatype* t = stack->pop();
			if (t == nullptr) throw InvalidDestination(addr);
			ins_ptr = t->dvalue;
			ins = ptr[ins_ptr];
			delete t;
		}

		}
		catch(out_of_range& e)
		{
			throw InvalidOperand(addr);
		}
		ins = ins->next;
		++ins_ptr;
	}
}

type CPU::getType(string& str)
{
	if (Util::isDec(str)) return type::INT;
	if (Util::isFlt(str)) return type::FLOAT;
	if (Util::isBoo(str)) return type::BOOL;
	if (Util::isAdr(str)) return type::ADDR;
	if (Util::isReg(str)) return loadReg(str).typ;
	return type::NONE;
}

datatype& CPU::loadReg(string& str)
{
	return registers[stoi(str.substr(1)) - 1];
}

datatype& CPU::loadMem(int addr)
{
	return sref[addr];
}

void CPU::clean()
{

};



/*========================================================================*/



void RStack::push(datatype& ele)
{
	stack->append(ele);
}

datatype* RStack::pop()
{
	return stack->pop_back();
}

int RStack::getStackSize()
{
	return stack->getSize();
}

bool RStack::isEmpty()
{
	return stack->getSize() == 0;
}

void RStack::clear()
{
	stack->clear();
}



/*========================================================================*/



template<typename T>
int List<T>::append(T& ele)
{
	if (!this->head and !this->tail)
		this->head = this->tail = &ele;
	else
	{
		this->tail->next = &ele;
		this->tail = &ele;
	}
	this->size++;
	return 0;
}

template<typename T>
int List<T>::getSize()
{
	return this->size;
}

template<typename T>
T* List<T>::pop_back()
{
	if (this->size <= 0) return nullptr;
	else
	{
		T* ret,*temp = this->head;
		if (this->head == this->tail)
		{
			ret = this->head;
			this->head = this->tail = nullptr;
			return ret;
		}
		while (temp->next != this->tail) temp = temp->next;
		ret = this->tail;
		this->tail = temp;
		this->tail->next = nullptr;
		this->size--;
		return ret;
	}
}

template<typename T>
T* List<T>::operator[](int idx)
{
	T* ptr = this->head;
	while(ptr)
	{
		if (ptr->addr == idx)
			return ptr;
		ptr = ptr->next;
	}
	return nullptr;
}

template<typename T>
void List<T>::clear()
{
	if (this->head == nullptr) return;
	T* temp = head;
	while(true)
	{
		T* next = temp->next;
		delete temp;
		if (next == nullptr) break;
		temp = next;
	}
}
