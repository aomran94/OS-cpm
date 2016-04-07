void doView(char*);
void doExecute(char*);
void doDelete(char*);
void doDir();
void doCopy(char*);
void doCreate(char*);
int cmdNumber(char*);
int MOD(int, int);
int DIV(int, int);

char shellBuf[13312];
char readBuf[80];

int main(){

	int cmdNum;
	int iShell;
	char cmd[80];

	while(1){

		for(iShell=0; iShell<13312; iShell++) shellBuf[iShell] = 0x0;
		interrupt(0x21, 0, "\nA:>\0", 0, 0);
		interrupt(0x21, 1, cmd, 0, 0);
		cmdNum = cmdNumber(cmd);

		if( cmdNum == 0 ) doView(cmd);
 		else if( cmdNum == 1 ) doExecute(cmd);
		else if( cmdNum == 2 ) doDelete(cmd);
		else if( cmdNum == 3 ) doDir();			
		else if( cmdNum == 4 ) doCopy(cmd);
		else if( cmdNum == 5 ) doCreate(cmd);
		else interrupt(0x21, 0, "Bad Command !!\0", 0, 0);
		
	}

	return 0;
}


void doView(char* CMD){
	
	int iShell;
	char fName1[7];
	
	iShell = 0;
	while( iShell<7 && *(CMD+5+iShell)!=0xD && *(CMD+5+iShell)!=' ' ) { *(fName1+iShell) = *(CMD+5+iShell); iShell++; }
	while( iShell<7 ) { *(fName1+iShell) = 0x00; iShell++; }

	interrupt(0x21, 3, fName1, shellBuf, 0);
	if( *shellBuf==0 ) interrupt(0x21, 0, "VIEW-ERROR 0xA1: Either file not exists or you've not specified a name. \0", 0, 0); 
	else interrupt(0x21, 0, shellBuf, 0, 0);

}

void doExecute(char* CMD){

	int iShell;
	char fName1[7];

	iShell = 0;
	while( iShell<7 && *(CMD+8+iShell)!=0xD && *(CMD+8+iShell)!=' ' ) { *(fName1+iShell) = *(CMD+8+iShell); iShell++; }
	while( iShell<7 ) { *(fName1+iShell) = 0x00; iShell++; }

	interrupt(0x21, 4, fName1, 0x2000, 0);

	interrupt(0x21, 0, "EXECUTE-ERROR 0xA2: program not found.\0", 0, 0);

}

void doDelete(char* CMD){

	int iShell;
	char fName1[7];

	iShell = 0;
	while( iShell<7 && *(CMD+7+iShell)!=0xD && *(CMD+7+iShell)!=' ' ) { *(fName1+iShell) = *(CMD+7+iShell); iShell++; }
	while( iShell<7 ) { *(fName1+iShell) = 0x00; iShell++; }

	interrupt(0x21, 3, fName1, shellBuf, 0);
	if( *shellBuf==0x0 ) interrupt(0x21, 0, "DELETE-ERROR 0xA3: Either file not exists or you've not specified a name.\0", 0, 0);
	else {
		interrupt(0x21, 7, fName1, 0, 0);
		interrupt(0x21, 0, "File: \"\0", 0, 0);
		interrupt(0x21, 0, fName1, 0, 0);
		interrupt(0x21, 0, "\" deleted successfully. \0", 0, 0);
	}


}

void doDir(){

	int iShell;
	int dShell;
	int fileSize;
	char fName1[7];
	char fName2[7];
	char dirSect[512];

	interrupt(0x21, 2, dirSect, 2, 0);
	dShell = 0;

	while( dShell<512 ){

		if( *(dirSect+dShell)==0x0 ) { dShell+=32; continue; }


		for(iShell=0; iShell<7; iShell++) fName1[iShell]=0x0;

		*(fName1+0)=*(dirSect+dShell+0); *(fName1+1)=*(dirSect+dShell+1); *(fName1+2)=*(dirSect+dShell+2);
		*(fName1+3)=*(dirSect+dShell+3); *(fName1+4)=*(dirSect+dShell+4); *(fName1+5)=*(dirSect+dShell+5);
		*(fName1+6)=0x0;
		
		interrupt(0x21, 0, fName1, 0, 0);
		interrupt(0x21, 0, " :: Size = ", 0, 0);

		fileSize = 0; iShell=6;
		while( iShell<32 && *(dirSect+dShell+iShell)!=0x00 ) { fileSize++; iShell++; }

		*fName2 = (DIV(fileSize, 10)) + 48;
		*(fName2+1) = (MOD(fileSize, 10)) + 48;
		*(fName2+2) = '\0';

		interrupt(0x21, 0, fName2, 0, 0);
		interrupt(0x21, 0, " Sectors. \n\0", 0, 0);

		dShell+=32;
	}
}

void doCopy(char* CMD){

	int iShell;
	int iShelltmp;
	int fileSize;
	char fName1[7];
	char fName2[7];

	iShell = 0;
	while( iShell<7 && *(CMD+5+iShell)!=0xD && *(CMD+5+iShell)!=' ' ) { *(fName1+iShell) = *(CMD+5+iShell); iShell++; }
	iShelltmp = iShell;
	while( iShelltmp<7 ) { *(fName1+iShelltmp) = 0x00; iShelltmp++; }

	if( *(CMD+5+iShell)!=' ' ) 
		{ interrupt(0x21, 0, "COPY-ERROR 0xA5: Wrong CMD format, It should be \"copy fName1 fName2\".\0", 0, 0); return; }

	iShell++;
	iShelltmp = 0;

	while( iShelltmp<7 && *(CMD+5+iShell+iShelltmp) != 0xD && *(CMD+5+iShell+iShelltmp) != ' ' )
		{ *(fName2+iShelltmp) = *(CMD+5+iShell+iShelltmp); iShelltmp++; }
	
	if( iShelltmp==0 ) { 
		interrupt(0x21, 0, "COPY-ERROR 0xA5: Wrong CMD format, It should be \"copy fName1 fName2\".\0", 0, 0); return;
	}

	while( iShelltmp<7 ) { *(fName2+iShelltmp) = 0x00; iShelltmp++; }

	

	interrupt(0x21, 3, fName1, shellBuf, 0);

	for(iShell=0; iShell<26; iShell++){
		for(iShelltmp=0; iShelltmp<512; iShelltmp++){
			if( *(shellBuf+(iShell*512)+iShelltmp) != 0x0 ) break;
		}
		if(iShelltmp==512) break;
	}

	if( iShell==0 ) {
		interrupt(0x21, 0, "COPY-ERROR 0xA5: The file you want to copy does not exist.", 0, 0); return;
	}

	interrupt(0x21, 8, fName2, shellBuf, iShell);

	interrupt(0x21, 0, "File: \"\0", 0, 0);
	interrupt(0x21, 0, fName1, 0, 0);
	interrupt(0x21, 0, "\" copied successfully to file \"\0", 0, 0);
	interrupt(0x21, 0, fName2, 0, 0);
	interrupt(0x21, 0, "\".\0", 0, 0);


}

void doCreate(char* CMD){

	int iShell;
	int iShelltmp;
	int secNum;
	char fName1[7];
	
	iShell=0;
	while( iShell<7 && *(CMD+7+iShell)!=0xD && *(CMD+7+iShell)!=' ' ) { *(fName1+iShell) = *(CMD+7+iShell); iShell++; }
	while( iShell<7 ) { *(fName1+iShell) = 0x00; iShell++; }

	iShell=0;
	
	while(1){
		
		for(iShelltmp=0; iShelltmp<80; iShelltmp++) *(readBuf+iShelltmp) = 0x0;
		
		interrupt(0x21, 0, ">\0", 0, 0);
		interrupt(0x21, 1, readBuf, 0, 0);
		
		if( *readBuf == 0xD ) break;

		iShelltmp=0;
		while( *(readBuf+iShelltmp) != 0x0 ){
			*(shellBuf+iShell) = *(readBuf+iShelltmp);
			iShell++; iShelltmp++;
		}
		
	}

	secNum = DIV(iShell, 512) + 1;
	interrupt(0x21, 8, fName1, shellBuf, secNum);

	interrupt(0x21, 0, "File written successfully to disk. \"\0", 0, 0);

}

int cmdNumber(char* CMD){

	if( *(CMD)== 'v' && *(CMD+1)== 'i' && *(CMD+2)== 'e' && *(CMD+3)== 'w' && *(CMD+4)== ' ' ){
		return 0;		// A:>view [fileName1]
	}

	if( *(CMD)== 'e' && *(CMD+1)== 'x' && *(CMD+2)== 'e' && *(CMD+3)== 'c' && *(CMD+4)== 'u'
						&& *(CMD+5)== 't' && *(CMD+6)== 'e' && *(CMD+7)== ' ' ){
		return 1;		// A:>execute [fileName1]
	}

	if( *(CMD)== 'd' && *(CMD+1)== 'e' && *(CMD+2)== 'l' && *(CMD+3)== 'e' && *(CMD+4)== 't'
						&& *(CMD+5)== 'e' && *(CMD+6)== ' ' ){
		return 2;		// A:>delete [fileName1]
	}

	if( *(CMD)== 'd' && *(CMD+1)== 'i' && *(CMD+2)== 'r' && *(CMD+3)== 0xD ){
		return 3;		// A:>dir
	}

	if( *(CMD)== 'c' && *(CMD+1)== 'o' && *(CMD+2)== 'p' && *(CMD+3)== 'y' && *(CMD+4)== ' ' ){
		return 4;		// A:>copy [fileName1] [fileName2]
	}

	if( *(CMD)== 'c' && *(CMD+1)== 'r' && *(CMD+2)== 'e' && *(CMD+3)== 'a' && *(CMD+4)== 't'
						&& *(CMD+5)== 'e' && *(CMD+6)== ' ' ){
		return 5;		// A:>create [fileName1]
	}

	return -1;

}

int MOD(int x, int y){
	if(x<y) return x;
	else return MOD(x-y, y);
}

int DIV(int x, int y){
	if(x<y) return 0;
	else return (1+DIV(x-y,y));
}