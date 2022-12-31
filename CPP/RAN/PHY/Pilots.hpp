#pragma once 

#include "Headers.hpp"

class LFSR_for_GoldSequence{
	public:
	LFSR_for_GoldSequence()
	:m_index(0), m_init(0), m_var1(1), m_var2(0)
	{
	};

	~LFSR_for_GoldSequence(){
	};
	
	
	void getPRBSbits(unsigned int w_init,int* data, int w_size)
	{
		this->initTo(w_init);
		
		this->getBits(data, w_size);
		
	};

	
	void initTo(unsigned int w_init)
	{
	
		if(w_init<31)
		{
				throw std::invalid_argument("LFSR_for_GoldSequence initialization shouldd have at least length 31");
		}
	
	int Nc = 1600;

	m_var1 = 1;
	m_var2 = w_init;
	unsigned long int tmp1, tmp2;
	tmp1 = 0;
	tmp2 = 0;
	// rund throguh Nc samples to get into initial step 
	for(unsigned int i1 = 0; i1 < (Nc-31); i1 ++)
	{
		tmp1 = ((m_var1 & 1) ^ ((m_var1 >> 3) & 1));
		m_var1 ^= (tmp1 << 31);
		m_var1 >>= 1;
		tmp2 = ((m_var2 & 1) ^ ((m_var2 >> 1) & 1) ^ ((m_var2 >> 2) & 1) ^ ((m_var2 >> 3) & 1));
		m_var2 ^= (tmp2 << 31);
		m_var2 >>= 1;
	}
	}
	void getBits(int* data, int w_size)
	{
		
	m_size = w_size;
	unsigned char *m_bits = new unsigned char[w_size];
	memset(m_bits,0,w_size*sizeof(char));
	
	unsigned long int tmp1, tmp2, tmp_bit;
	tmp1=0;
	tmp2=0;
	tmp_bit=0;
	// generates the actual samples 
	for(unsigned int i1 = 0; i1 < (m_size - 1); i1++)
	{
		tmp1 = ((m_var1 & 1) ^ ((m_var1 >> 3) & 1));
		m_var1 ^= (tmp1 << 31);
		m_var1 >>= 1;
		tmp2 = ((m_var2 & 1) ^ ((m_var2 >> 1) & 1) ^ ((m_var2 >> 2) & 1) ^ ((m_var2 >> 3) & 1));
		m_var2 ^= (tmp2 << 31);
		m_var2 >>= 1;		
		tmp_bit = ((tmp1&1)^(tmp2&1)&1);
		//std::cout<<(unsigned long int)tmp2<<" ";
		data[i1]=(int)tmp_bit;
//		std::cout<<(int)tmp_bit<<" ";
	}
//	std::cout<<std::endl;

	m_index = 0;
/* 
	std::ofstream myfile3;
	myfile3.open("PSS.dat", std::ios::binary | std::ios::out);	
	//myfile.write((char*)&dl_buff_collected[0],sizeof(cFloat)*tmpSamps);
	myfile3.write((char*)&m_syncInF[0],sizeof(cFloat)*m_fftSize);
	myfile3.close(); 
*/

	};
	
	public:
	unsigned int m_size;
	unsigned long int m_init;
	unsigned long int m_var1, m_var2;
	unsigned int m_index;
	
};

/***********************************************************************
 * Class Pilots
 **********************************************************************/

class Pilots{
	public:
	Pilots(){

		m_pilots = new cFloat*[m_number_of_pilot_symbols];
		m_pilots_location = new int*[m_number_of_pilot_symbols];
		for(int i1=0;i1<m_number_of_pilot_symbols;i1++)
		{
			m_pilots[i1] = new cFloat[2*110];

			m_pilots_location[i1] = new int[2*110];
			memset(m_pilots[i1],0,2*110*sizeof(cFloat));
		        memset(m_pilots_location[i1],0,2*110*sizeof(int));
		}
				
	};
	~Pilots(){
		
		for(int i1=0;i1<m_number_of_pilot_symbols;i1++)
		{
			delete(m_pilots[i1]);
			delete m_pilots_location[i1];
		}
		delete m_pilots;
		delete m_pilots_location;
	};
	
	int generateAllPilots(int w_cell_id, int w_NRBDL)
	{	

		m_cell_id = w_cell_id;
		m_NRBDL = w_NRBDL;
		for(int i1=0;i1<m_number_of_pilot_symbols;i1++)
		{
			delete(m_pilots[i1]);
			delete(m_pilots_location[i1]);
		}
		for(int i1=0;i1<m_number_of_pilot_symbols;i1++)
		{
			m_pilots[i1] = new cFloat[2*m_NRBDL];
			m_pilots_location[i1] = new int[2*m_NRBDL];
		  memset(m_pilots[i1],0,2*m_NRBDL*sizeof(cFloat));
		  memset(m_pilots_location[i1],0,2*m_NRBDL*sizeof(int));
		}

	int p=0;
	int i1 =0;
	for(int inx_sf = 0;inx_sf < m_number_of_subframes; inx_sf++)
	{
		for(int inx_slot = 0;inx_slot < m_number_of_slots; inx_slot++)
		{
			for(int inx_pilots_sym = 0;inx_pilots_sym < m_number_of_pilot_symbols_in_slot; inx_pilots_sym++)
			{
				int tmp_inx = inx_sf*m_number_of_slots*m_number_of_pilot_symbols_in_slot + inx_slot*m_number_of_pilot_symbols_in_slot + inx_pilots_sym;
				
				// std::cout<< tmp_inx<<std::endl;
				// prepare the cell config
				int slot_number = inx_sf*m_number_of_slots + inx_slot;
				int l = m_pilot_symbols[inx_pilots_sym];
				int cInit = (1<<10)*(7*(slot_number+1)+l+1)*(2*m_cell_id+1)+2*m_cell_id + m_NCP;
				
				// get pilots for symbols in this slot
				this->generatePilotsOfOneSymbol(cInit, m_NRBDL, m_pilots[i1]);
				
				// get positions of the pilot symbols 
				// k = 6*m +mod(nu+nushift,6);
				
				int nu = 3;
				if( (p == 0) & (l==0))
					nu = 0;
					
				for(int i2 =0; i2<(2*m_NRBDL);i2++)
					{
						int tmpLoc = 6*i2 + (nu + m_nushift)%6;
						if(i2<m_NRBDL)
							m_pilots_location[i1][i2] = m_fftSize-m_NRBDL/2*12+tmpLoc;
						else
							m_pilots_location[i1][i2] = (tmpLoc-m_NRBDL/2*12+1);
							
					}
				
				i1++;
				//std::cout<<cInit<<std::endl;
			
			// std::cout<<"case:"<<inx_sf<<" "<<inx_slot<<" "<<inx_pilots_sym<<std::endl;
			} 
		} 
	} 
	
	/*
	for(int i1 = 0;i1<2*m_NRBDL;i1++)
		std::cout<<m_pilots[0][i1]<<" ";
		std::cout<<std::endl;

	for(int i1 = 0;i1<2*m_NRBDL;i1++)
		std::cout<<m_pilots_location[0][i1]<<" ";
		std::cout<<std::endl;
	*/
	
		m_pilots_generated = true;

		return 0;
	};
	
	int generatePilotsOfOneSymbol(int w_cInit, int w_NRBDL, cFloat *o_pilots)
	{
		
		m_lfsr.getPRBSbits(w_cInit, &m_prbs[0], m_Mpn+1);
		
		cFloat* rlnskru = new cFloat[m_Mpn/2];
		memset(rlnskru,0,m_Mpn/2*sizeof(cFloat));

		float sqrt2 = std::sqrt(1/2.0);
		for(int i1 =0;i1<m_Mpn/2;i1++)
		{
			rlnskru[i1] = cFloat((1-2*m_prbs[2*i1]),-1*(1-2*m_prbs[2*i1+1]));
		}
		
		// cFloat* tmprs = new cFloat[2*w_NRBDL];
	
		for(int i1 = 0; i1<2*w_NRBDL;i1++)
		{
			o_pilots[i1] = rlnskru[m_NmaxRB-w_NRBDL+i1];
		}
		
		return 0;
	};
	
	cFloat* getPilotsForTheSymbol(int w_frame_number,int w_slot_number, int w_pilot_symbol_l)
	{
		
		
		//std::cout<<"get pilots:"<<w_frame_number<<" "<<w_slot_number<<" "<<w_pilot_symbol_l<<std::endl;

		if(m_pilots_generated==true)
		{
			int inx = w_frame_number*2*2+w_slot_number*2 + w_pilot_symbol_l;
			//std::cout<<inx<<std::endl;
			return &m_pilots[inx][0];
		}
		return 0;
	};
	
	int* getPilotsLocationsForTheSymbol(int w_frame_number,int w_slot_number, int w_pilot_symbol_l)
	{
		
		
		//std::cout<<"get pilots:"<<w_frame_number<<" "<<w_slot_number<<" "<<w_pilot_symbol_l<<std::endl;

		if(m_pilots_generated==true)
		{
			int inx = w_frame_number*2*2+w_slot_number*2 + w_pilot_symbol_l;
			//std::cout<<inx<<std::endl;
			return &m_pilots_location[inx][0];

		}
		return 0;
	};
		

	
	public:
	
	int m_fftSize = 1024;
	int m_NsymbDL = 7;
	int m_pilot_symbols[2] = {0, m_NsymbDL-3};
	int m_subframe_number;
	int m_slot_number;
	
	bool m_pilots_generated = false;
	int m_N_id_1 = 0;
	int m_N_id_2 = 0;

	int m_cell_id;
	int m_nushift;
	int m_NmaxRB = 110;
	int m_NRBDL = 50;
	int m_NCP = 1;
	int m_Mpn  = 440;//110*2+110*2;
	int *m_prbs = new int[m_Mpn];
	
	// helping functions 
	int m_number_of_pilot_symbols_in_slot = 2;
	int m_number_of_slots = 2;
	int m_number_of_subframes = 10;
	
	int m_number_of_pilot_symbols = m_number_of_pilot_symbols_in_slot*m_number_of_slots*m_number_of_subframes;
	cFloat **m_pilots;
	int **m_pilots_location;

	LFSR_for_GoldSequence m_lfsr;
};

