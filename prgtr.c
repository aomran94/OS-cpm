int main(){

	interrupt(0x21, 0, "Tagrooba\0", 0, 0);
	interrupt(0x21, 0, "Tagrooba2\0", 0, 0);
	interrupt(0x21, 5, 0, 0, 0);
	
}