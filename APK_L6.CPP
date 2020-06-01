#include <stdio.h>
#include <dos.h>
#include <stdlib.h>
#include <string.h>


//Before any write to input register, need to make sure it's free
char waitForReg()
{
	int triesNum = 1000;
	while(--triesNum > 0)
	{
		if((inp(0x64) & 0x02) == 0)
			break;
	}
	if(triesNum <= 0)
		return 0;
	return 1;
}

//Send byte to input register. If failed after 3 tries - error, return false
char trySendByte(char byte)
{
	int i;
	for(i = 0;i <= 3;i++)
	{
		if(waitForReg() == 0)
			return 0;
		outp(0x60, byte);
		if(inp(0x60) == 0xFA)
			break;
	}
	if(i == 3)
		return 0;
	return 1;
}



char blinkLights()
{
	int i;
	char noError = 1;
	for(i = 0;i < 10;i++)
	{
		//Display 3-bit binary count 10 times
		char j;
		for(j = 0;j < 8;j++)
		{
			//Send "access indicators" command byte
			if(0 == trySendByte(0xED))
				noError = 0;
			//Set indicators
			if(0 == trySendByte(j))
				noError = 0;
			delay(50);
		}
	}
	return noError;
}


void interrupt (*defKeybInt)(...);
void interrupt newKeybInt(...)
{
	disable();
	//Get key code, put on screen directly

	char keyMsg[] = "Key code: 0x";
	char keyCode[3] = {0};

	//Get address of video memory
	unsigned far *screen = (unsigned far*)MK_FP(0xB800, 0);

	char key = inp(0x60);	//read the input
	itoa(key, keyCode, 16);	//make it into HEX string

	short charAttr = 0x6A00;

	int i;
	//Print base message
	for(i = 0;i<strlen(keyMsg);i++)
	{
		short consChar = charAttr | keyMsg[i];

		*screen = consChar;
		screen++;
	}
	//Print key code in HEX
	for(i = 0;i<strlen(keyCode);i++)
	{
		short consChar = charAttr | keyCode[i];

		*screen = consChar;
		screen++;
	}

	defKeybInt();

	enable();
}



int main()
{
	//Attach keyboard sniffer thing


	defKeybInt = getvect(0x09);
	setvect(0x09, newKeybInt);

	printf("Keyboard actions will be displayed for 10 seconds.\n");
	delay(10000);
	printf("Delay ended. Started keyboard blinker.\n");

	setvect(0x09, defKeybInt);

	if(blinkLights() == 0)		//WORKS!!!!!!!
	{
		printf("Error during blinking! \n");
		return -1;
	}

	printf("done \n");

	return 0;
}