/*
 *  Copyright (C) 2002  The DOSBox Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "dos_inc.h"

class device_CON : public DOS_Device {
public:
	device_CON();
	bool Read(Bit8u * data,Bit16u * size);
	bool Write(Bit8u * data,Bit16u * size);
	bool Seek(Bit32u * pos,Bit32u type);
	bool Close();
	Bit16u GetInformation(void);
private:
	Bit8u cache;
};
void INT10_TeletypeOutput(Bit8u chr,Bit8u attr,bool showattr, Bit8u page);

bool device_CON::Read(Bit8u * data,Bit16u * size) {
	Bit16u oldax=reg_ax;
	Bit16u count=0;
	if ((cache) && (*size)) {
		data[count++]=cache;
        if(dos.echo) {
            INT10_TeletypeOutput(cache,7,false,0);
        }
        cache=0;

	}
	while (*size>count) {
		reg_ah=0;
		CALLBACK_RunRealInt(0x16);
		switch(reg_al) {
		case 13:
			data[count++]=0x0D;
			if (*size>count) data[count++]=0x0A;    // it's only expanded if there is room for it. (NO cache)
  			*size=count;
			reg_ax=oldax;
			return true;
            break;
        case 8:
            if(*size==1) data[count++]=reg_al;  //one char at the time so give back that BS
            else if(count) {                    //Remove data if it exists (extended keys don't go right)
                data[count--]=0;
                INT10_TeletypeOutput(8,7,false,0);
                INT10_TeletypeOutput(' ',7,false,0);
            } else {
                continue;                       //no data read yet so restart whileloop.
            }
                
            break;
		default:
			data[count++]=reg_al;
			break;
		case 0:
			data[count++]=reg_al;
			if (*size>count) data[count++]=reg_ah;
			else cache=reg_ah;
			break;
            
            }
        if(dos.echo) { //what to do if *size==1 and character is BS ?????
            INT10_TeletypeOutput(reg_al,7,false,0);
        }
	}
	*size=count;
	reg_ax=oldax;
	return true;
}


bool device_CON::Write(Bit8u * data,Bit16u * size) {
	Bit16u count=0;
	while (*size>count) {

		INT10_TeletypeOutput(data[count],7,false,0);
		count++;
	}
	*size=count;
	return true;
}

bool device_CON::Seek(Bit32u * pos,Bit32u type) {
	// seek is valid
	*pos = 0;
	return true;
}

bool device_CON::Close() {
	return false;
}

Bit16u device_CON::GetInformation(void) {
	Bit16u head=mem_readw(BIOS_KEYBOARD_BUFFER_HEAD);
	Bit16u tail=mem_readw(BIOS_KEYBOARD_BUFFER_TAIL);
	
	if ((head==tail) && !cache) return 0x80D3; /* No Key Available */
	return 0x8093;		/* Key Available */
};


device_CON::device_CON() {
	name="CON";
	cache=0;
}

