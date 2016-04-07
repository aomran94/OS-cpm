/* 
 * Prototypes Definition
 */

void printString(char*);
void readString(char*);

int MOD(int, int);
int DIV(int, int);

void readSector(char*, int);
void writeSector(char*, int);

void readFile(char*, char*);
void writeFile(char* name, char* buffer, int secNum);
void deleteFile(char* name);

void executeProgram(char*, int);
void terminate();

void handleInterrupt21(int, int, int, int);

main(){
	
	char A[13312];
	makeInterrupt21();
	
	
	interrupt(0x21, 4, "shell\0", 0x2000, 0); // Run the shell
	
	while(1);

}

void printString(char* text){
	int i = 0; 
	char c;
	while( *(text+i) != 0x0 ){
		c = *(text+i);
		if( c == 0xA ) { interrupt(0x10, 0xE*256+(0xD), 0, 0, 0); }

		interrupt(0x10, 0xE*256+(c), 0, 0, 0);

		if( c == 0xA ) { interrupt(0x10, 0xE*256+(0x8), 0, 0, 0); } 

		i++;
	}
}


void readString(char* text){
	int size;
	char c;
	char* ptr;

	size = 80;
	ptr = text;
	while( size && c != 0xD ){

		c = interrupt(0x16, 0x0, 0, 0, 0);

		if( c != 0x8 ){
			interrupt(0x10, 0xE*256+c, 0, 0, 0);
			*ptr++ = c;
			size--;

		} else {
			if( ptr != text ){ 
				interrupt(0x10, 0xE*256+c, 0, 0, 0);
				interrupt(0x10, 0xE*256+0x20, 0, 0, 0);
				interrupt(0x10,0xE*256+0x8,0,0,0);
				ptr--;
				size++;
			}
		}

	}

	if( c == 0xD ){
		*ptr = 0xA;
	} else {
		interrupt(0x10, 0xE*256+0xD, 0, 0, 0);
		*ptr = 0xD;
		*++ptr = 0xA;
	}

	interrupt(0x10, 0xE*256+(*ptr), 0, 0, 0);
	*++ptr=0x0;
}

int MOD(int x, int y){
	if(x<y) return x;
	return MOD(x-y, y);
}

int DIV(int x, int y){
	if(x<y) return 0;
	return (1+DIV(x-y,y));
}

void readSector(char* buffer, int sector){
	int rvSector;
	int head;
	int track;
	rvSector = (MOD(sector, 18) + 1);
	head = MOD(DIV(sector, 18), 2);
	track = DIV(sector, 36);

	interrupt(0x13, 0x2*256+(0x1), buffer, (track)*256+rvSector, (head)*256+0x0);
}

void writeSector(char* buffer, int sector){
	int rvSector;
	int head;
	int track;
	rvSector = (MOD(sector, 18) + 1);
	head = MOD(DIV(sector, 18), 2);
	track = DIV(sector, 36);
	
	interrupt(0x13, 0x3*256+(0x1), buffer, (track)*256+rvSector, (head)*256+0x0);
}

void readFile(char* fileName, char* buffer){
	int iDir;
	int i;
	int j;
	int zPos;
	char dirBuffer[512];

	readSector(dirBuffer, 2);

	//interrupt(0x21, 0, fileName, 0, 0);
	
	iDir=0;

	if( *fileName==0 ) { interrupt(0x21, 0, "ERROR 0x05: No entered File Name.\0", 0, 0); return; }

	zPos = 0;
	while( *(fileName+zPos) != 0x0 ) zPos++;
	if( zPos > 6 ) iDir = 512; // Since there doesNOT exist any file whose name > 6 characters. No need for search.
	

	while(iDir<512){
		i=0; 
		
		while( i<zPos ){
			if(*(fileName+i)!=*(dirBuffer+iDir)) break;
			iDir++;
			i++;
		}

		if( i==zPos ){
			while( i<6 ) {
				if( *(dirBuffer+iDir) != 0x0 ) break;
				iDir++;
				i++;
			}
			if( i!=6 ) { iDir=(iDir-i) + 32; continue; }
		} else { iDir=(iDir-i) + 32; continue; }

		
		for(i=0; i<26; i++){
			if(*(dirBuffer+iDir+i) != 0) {
				readSector(buffer, *(dirBuffer+iDir+i));
				buffer += 512;
			} else break;
		}
		
		while( i<26 ){
			for(j=0; j<512; j++) *(buffer+j) = 0x0;

			buffer += 512;
			i++;
		}
		return;

		
	}


	if( iDir==512 ){ interrupt(0x21, 0, "ERROR 0x04: Cannot Find a file with the name specified to read.\0", 0, 0); return; }

}

void writeFile(char* name, char* buffer, int secNum){
	char mapSector[512];
	char dirSector[512];
	char emptySectors[26];
	int iMap;
	int iFile;
	int iPerFile;
	int iSec;
	
	readSector(mapSector, 1);
	readSector(dirSector, 2);
 
	if( secNum > 26 ) { interrupt(0x21, 0, "ERROR 0x00: FileSystem Cannot Support >26 Sectors Files.\0", 0, 0); return; }
	
	/* This will try to find a number of empty sectors equivalent to "secNum" and  
	 * stores their addresses in an array called "emptySectors"
	 */
	iSec = 0; iMap = 0;
	while( iSec<secNum ){
		// Try to find an empty Sector
		while( iMap<512 && *(mapSector+iMap)!=0x00 )
			iMap++;
    
		// If not able to find one and still it's needed; print an error and return
		if( iMap==512 ) { interrupt(0x21, 0, "ERROR 0x01: No Disk Space for your file.\0", 0, 0); return; }
		else { *(emptySectors + iSec) = (iMap); iSec++; iMap++; }
	}

	/* This will make "j" point to the first empty Directory entry in 
	 * the dirSector; if none exists it'll stop at 512
	 */
	iFile = 0;
	while( iFile<512 && *(dirSector + iFile)!=0x00 )
		iFile += 32;
	if( iFile >= 512 ) { interrupt(0x21, 0, "ERROR 0x02: Current FileSystem Cannot support more files (>13).\0", 0, 0); return; }

	// This will write the filename to the directory entry
	iPerFile = 0;
	while ( iPerFile<6 && *(name+iPerFile)!='\0')
		*((dirSector+iFile) + iPerFile++) = *(name + iPerFile);
	while ( iPerFile<6 ) 
		*((dirSector+iFile) + iPerFile++) = 0x00;

  

	// This will write the File sectors to the disk, update the Directory entry and mapSectors
	iSec = 0;
	while( (iPerFile-6) < secNum ) {
		*((dirSector + iFile) + iPerFile) = *(emptySectors + iSec);
		*(mapSector + ((*(emptySectors + iSec)) - 1)) = 0xFF;
		writeSector(buffer, *(emptySectors + iSec));
		iPerFile++;
		iSec++;
	}

	// Write back the updated Directory and Map sectors
	writeSector(mapSector, 1);
	writeSector(dirSector, 2);

}

void deleteFile(char* name){
	char mapSector [512];
	char dirSector [512];

	int iDir;
	int i;
	int zPos;

	iDir=0;

	readSector(mapSector, 1);
	readSector(dirSector, 2);

	if( *name==0 ) { interrupt(0x21, 0, "ERROR 0x05: No entered File Name.\0", 0, 0); return; }

	zPos = 0;
	while( *(name+zPos) != 0x0 ) zPos++;
	if( zPos > 6 ) iDir = 512; // Since there doesNOT exist any file whose name > 6 characters. No need for search.

	while( iDir<512 ){

		i=0;
		while( i<zPos ){
			if(*(name+i)!=*(dirSector+iDir+i)) break;
			i++;
		}

		if( i==zPos ){
			while( i<6 ) {
				if( *(dirSector+iDir+i) != 0x0 ) break;
				i++;
			}
			if( i!=6 ) { iDir += 32; continue; }
		} else { iDir += 32; continue; }

		
		*(dirSector+iDir) = 0x0;

		for(i=0; i<26; i++){
			if( *(dirSector+iDir+6+i) != 0x00 ) { *( mapSector + *(dirSector+iDir+6+i) ) = 0x00; }
			else break;
		}
		
		break;
		
	}

	if( iDir==512 ){ interrupt(0x21, 0, "ERROR 0x03: Cannot Find a file with the name specified to delete.\0", 0, 0); return; }
	
	writeSector(mapSector, 1);
	writeSector(dirSector, 2);

}

void executeProgram(char* name, int segment){
	char progBuffer[13312];
	int ptr = 0;

	for(ptr=0; ptr<13312; ptr++) progBuffer[ptr] = 0x0;

	ptr = 0;

	readFile(name, progBuffer);
	if( *progBuffer==0x0 ) { interrupt(0x21, 0, "EXECUTE-ERROR 0xA2: program not found.\0", 0, 0); return; }

	while( ptr<13312 ) putInMemory(segment, ptr++, *(progBuffer+ptr));
	
	launchProgram(segment);
}

void terminate(){
	char shell[6];
	shell[0] = 's';
	shell[1] = 'h';
	shell[2] = 'e';
	shell[3] = 'l';
	shell[4] = 'l';
	shell[5] = '\0';
	interrupt(0x21, 4, shell, 0x2000, 0);
}

void handleInterrupt21(int ax, int bx, int cx, int dx){
	if(ax==0) printString(bx);
	else if(ax==1) readString(bx);
	else if(ax==2) readSector(bx,cx);
	else if(ax==3) readFile(bx,cx);
	else if(ax==4) executeProgram(bx,cx);
	else if(ax==5) terminate();
	else if(ax==6) writeSector(bx,cx);
	else if(ax==7) deleteFile(bx);
	else if(ax==8) writeFile(bx,cx,dx);
	else printString("ERROR 0xFF: Undefined Interrupt !!\0");
}