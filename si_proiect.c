#include <time.h> 
#include <stdio.h>
#include <hidef.h> // common defines and macros 
#include <stdlib.h>  
#include "derivative.h" // derivative-specific definitions 
          

#define COMPUTER 1 
#define HUMAN 2 

#define PLAYER1 3
#define PLAYER2 4
  
#define SIDE 3 // Lungimea tablei de joc
  
#define COMPUTERMOVE 'O' // Calculatorul va completa cu '0'
#define HUMANMOVE 'X' // Omul va completa cu 'X'

void MSDelay(unsigned int);

void newLine();
void sendString(char s[]);
void transmitereSeriala(unsigned char);
void sendStringWithoutNewLine(char s[]);

char scanKeypad(); 
void showBoard(char board[][SIDE]);
void showInstructions();
void initialise(char board[][SIDE]);

int rowCrossed(char board[][SIDE]);
int columnCrossed(char board[][SIDE]);
int diagonalCrossed(char board[][SIDE]);
int gameOver(char board[][SIDE]);

void playWithComputer(int whoseTurn);
void playWithFriend(int whoseTurn);

const unsigned char keypad[4][4] = // Initializarea tastaturii cu caractere
{
'1','2','3','A',
'4','5','6','B',
'7','8','9','C',
'*','0','#','D'
};

unsigned char column, row, choice;

unsigned int nrJocuri, winA = 0, winB = 0;

void main(void) 
{
  unsigned int i = 0;
 	 SCI0BDH = 0x00;
 	 SCI0BDL = 26;
 	 SCI0CR1 = 0x00;
 	 SCI0CR2 = 0x0C;

 	 //The RUN mode (SW7=10) of Dragon12+ works at 8MHz.
   //Serial Monitor used for LOAD works at 48MHz  
   //8MHz/2=4MHz, 4MHz/16=250,000 and 250,000/9600=26 
                         
   DDRA = 0x0F;                           // MAKE ROWS INPUT AND COLUMNS OUTPUT
   
   while(1) // Afisarea meniului
   {
      newLine();
      sendString("How many games you want to play? ");
      nrJocuri = scanKeypad();   // citim nr de jocuri
      if(nrJocuri >= '1' && nrJocuri <= '9')
        nrJocuri = nrJocuri - '0';//transformam in int
      newLine();
      sendString("A) PLAY WITH COMPUTER");
      newLine();
      sendString("B) PLAY WITH FRIEND");
      newLine();
   
      while(1)
      {
        sendString("Choose the opponent: ");
        choice = scanKeypad(); // Pentru jocul cu calculatorul, se asteapta apasarea tastei 'A'
        if(choice == 'A') 
        {
          while(i < nrJocuri){
              playWithComputer(HUMAN);
              i++;
          }
          break;
        }
        else if(choice == 'B') // Pentru jocul cu calculatorul, se asteapta apasarea tastei 'B'
        {
          while(i < nrJocuri){
            playWithFriend(PLAYER1);
            i++;
          }
          break;
        }
        else
          sendString("You must choose A or B!");
      }
      break;
   }  
       
}   

// Functie care genereaza timpul de asteptare
void MSDelay(unsigned int itime)
{
    unsigned int i, j;
    for(i=0; i<itime; i++)
    	for(j=0; j<4000; j++);
}

// Functie pentru transmiterea seriala a unui caracter
void transmitereSeriala(unsigned char c) // SCI0 
{
    while(!(SCI0SR1 & 0x80)); // Make sure the last bit is gone before giving it another byte	
    SCI0DRL = c;		      
}

// Use the pair (CR+LF) as a new-line character to send to Tera Term
// ASCII code 13 (decimal) is for CR, and 10 is for LF.
void newLine()
{
  transmitereSeriala(0x0D);
  transmitereSeriala(0x0A); 
}

// Functie care afiseaza un string, cu new-line
void sendString(char s[])
{
	int i =0;
	while (s[i] != 0x00) // Se desparte string-ul in caractere
	{
		transmitereSeriala(s[i]); // Se transmite serial fiecare caracter din string
		i++;
	}         
  newLine();
}

// Functie care afiseaza un string, fara new-line
void sendStringWithoutNewLine(char s[])
{
  int i =0;
  while (s[i] != 0x00)
  {
    transmitereSeriala(s[i]);
    i++;
  }         
}

char scanKeypad()
{                                             
   DDRA = 0x0F;                           // MAKE ROWS INPUT AND COLUMNS OUTPUT
   
   while(1)
   {                              
      do
      {                                 
         PORTA = PORTA | 0x0F;            // COLUMNS SET HIGH
         row = PORTA & 0xF0;              // READ ROWS
      } while(row == 0x00);               // WAIT UNTIL KEY PRESSED 

      do
      {                                 
         do
         {                              
            MSDelay(1);                   // WAIT
            row = PORTA & 0xF0;           // READ ROWS
         } while(row == 0x00);            // CHECK FOR KEY PRESS 
         
         MSDelay(15);                     // WAIT FOR DEBOUNCE
         row = PORTA & 0xF0;
      } while(row == 0x00);               // FALSE KEY PRESS 

      while(1)
      {                           
         PORTA &= 0xF0;                   // CLEAR COLUMN
         PORTA |= 0x01;                   // COLUMN 0 SET HIGH
         row = PORTA & 0xF0;              // READ ROWS
         if(row != 0x00)                  // KEY IS IN COLUMN 0
         {                 
            column = 0;
            break;                        
         }

         PORTA &= 0xF0;                   // CLEAR COLUMN
         PORTA |= 0x02;                   // COLUMN 1 SET HIGH
         row = PORTA & 0xF0;              // READ ROWS
         if(row != 0x00)                  // KEY IS IN COLUMN 1
         {                 
            column = 1;
            break;                        
         }

         PORTA &= 0xF0;                   // CLEAR COLUMN
         PORTA |= 0x04;                   // COLUMN 2 SET HIGH
         row = PORTA & 0xF0;              // READ ROWS
         if(row != 0x00)                  // KEY IS IN COLUMN 2
         {                 
            column = 2;
            break;                        
         }

         PORTA &= 0xF0;                   // CLEAR COLUMN
         PORTA |= 0x08;                   // COLUMN 3 SET HIGH
         row = PORTA & 0xF0;              // READ ROWS
         if(row != 0x00)                   // KEY IS IN COLUMN 3
         {                 
            column = 3;
            break;                        
         }
         row = 0;                         // KEY NOT FOUND
         break;                          
      }                                   

      if(row == 0x10)                     // KEY IS IN ROW 0
      {
         return keypad[0][column];         
      }
      else if(row == 0x20)                // KEY IS IN ROW 1
      {
         return keypad[1][column];
      }
      else if(row == 0x40)                // KEY IS IN ROW 2
      {
         return keypad[2][column];
 
      }
      else if(row == 0x80)                // KEY IS IN ROW 3
      {
         return keypad[3][column];
      }

      do
      {
         MSDelay(15);
         PORTA = PORTA | 0x0F;            // COLUMNS SET HIGH
         row = PORTA & 0xF0;              // READ ROWS
      } while(row != 0x00);               // MAKE SURE BUTTON IS NOT STILL HELD
   }                                      
}

// Functie care afiseaza tabla de joc
void showBoard(char board[][SIDE]) 
{ 
    newLine();
    newLine();
    
    sendStringWithoutNewLine("  ");
    transmitereSeriala(board[0][0]);
    sendStringWithoutNewLine(" | ");
    transmitereSeriala(board[0][1]);
    sendStringWithoutNewLine(" | ");
    transmitereSeriala(board[0][2]);
    sendString("  ");
    sendString("-------------"); 

    sendStringWithoutNewLine("  ");
    transmitereSeriala(board[1][0]);
    sendStringWithoutNewLine(" | ");
    transmitereSeriala(board[1][1]);
    sendStringWithoutNewLine(" | ");
    transmitereSeriala(board[1][2]);
    sendString("  ");
    sendString("-------------");

    sendStringWithoutNewLine("  ");
    transmitereSeriala(board[2][0]);
    sendStringWithoutNewLine(" | ");
    transmitereSeriala(board[2][1]);
    sendStringWithoutNewLine(" | ");
    transmitereSeriala(board[2][2]);
    sendString("  ");

    newLine();
    
    return; 
} 
  
// Functie care afiseaza instructiunile jocului
void showInstructions() 
{ 
    sendString("TIC-TAC-TOE"); 
    newLine();
    
    sendString("Choose a cell numbered from 1 to 9 as below:"); 
    newLine();
    
    sendString("  1 | 2 | 3  "); 
    sendString("-------------"); 
    sendString("  4 | 5 | 6  "); 
    sendString("-------------"); 
    sendString("  7 | 8 | 9  "); 
    newLine();
  
    return; 
}
   
// Functie care initializeaza jocul
void initialise(char board[][SIDE]) 
{ 
    int j,i;
    srand(1);  
      
    // Initial, tabla de joc este goala
    for (i=0; i<SIDE; i++) 
    { 
        for (j=0; j<SIDE; j++) 
            board[i][j] = ' '; 
    } 
    
    return; 
}

// Functie care verifica daca un jucator a completat o linie
int rowCrossed(char board[][SIDE]) 
{ 
    int i;
    for (i=0; i<SIDE; i++) 
    { 
        if (board[i][0] == board[i][1] && board[i][1] == board[i][2] &&  board[i][0] != ' ') 
          return 1; 
    } 
    return 0; 
} 
  
// Functie care verifica daca un jucator a completat o coloana
int columnCrossed(char board[][SIDE]) 
{ 
    int i;
    for (i=0; i<SIDE; i++) 
    { 
        if (board[0][i] == board[1][i] && board[1][i] == board[2][i] &&  board[0][i] != ' ') 
          return 1; 
    } 
    return 0; 
} 
  
// Functie care verifica daca un jucator a completat o diagonala
int diagonalCrossed(char board[][SIDE]) 
{ 
    if (board[0][0] == board[1][1] && board[1][1] == board[2][2] &&  board[0][0] != ' ') 
      return 1; 
          
    if (board[0][2] == board[1][1] && board[1][1] == board[2][0] && board[0][2] != ' ') 
      return 1; 
  
    return 0; 
} 
  
// Functie care verifica daca jocul a fost castigat de unul din jucatori 
int gameOver(char board[][SIDE]) 
{   
    // Jocul este castigat daca o linie, o coloana sau o diagonala este completata de acelasi jucator
    return(rowCrossed(board) || columnCrossed(board) || diagonalCrossed(board) ); 
}

// Functie pentru jocul om-calculator
void playWithComputer(int whoseTurn) 
{ 
    int moveIndex = 0, option, x, y, flag;
    char c; 
    
    // Tabla de joc este o matrice de caractere
    char board[SIDE][SIDE]; 
      
    // Initializarea jocului
    initialise(board); 
      
    // Afisarea instructiunilor
    showInstructions(); 
      
    // Jocul continua pana se ajunge la castigul unuia dintre jucatori sau egal (toata tabla este completata)
    while (gameOver(board) == 0 &&  moveIndex != SIDE*SIDE) 
    { 
        if (whoseTurn == COMPUTER) // Randul calculatorului
        { 
            while(1) // Se incearca gasirea unei mutari valide
            {
                option = rand() % 9 + 1; // Se genereaza o mutare random
                switch(option) // Se cauta locul din matrice corespunzator mutarii
                {
                    case 1: {
                        x = 0; 
                        y = 0; 
                        break; 
                    }
                    case 2: {
                        x = 0; 
                        y = 1; 
                        break; 
                    }
                    case 3: {
                        x = 0; 
                        y = 2; 
                        break; 
                    }
                    case 4: {
                        x = 1; 
                        y = 0; 
                        break; 
                    }
                    case 5: {
                        x = 1; 
                        y = 1; 
                        break; 
                    }
                    case 6: {
                        x = 1; 
                        y = 2; 
                        break; 
                    }
                    case 7: {
                        x = 2; 
                        y = 0; 
                        break; 
                    }
                    case 8: {
                        x = 2; 
                        y = 1; 
                        break; 
                    }
                    case 9: {
                        x = 2; 
                        y = 2; 
                        break; 
                    }
                }
                if(board[x][y] == ' ') // Daca locul este liber 
                {
                    board[x][y] = COMPUTERMOVE; // Se completeaza cu 0, altfel se genereaza mutari pana la gasirea unui loc liber
                    break;
                }
            }
            // Se afiseaza mutarea  
            sendStringWithoutNewLine("COMPUTER has put ");
            transmitereSeriala(COMPUTERMOVE);
            sendStringWithoutNewLine(" in cell ");
            c =  option + '0';
            transmitereSeriala(c);
            showBoard(board); // Se afiseaza tabla de joc actualizata
            moveIndex ++; // Creste numarul mutarilor
            whoseTurn = HUMAN; // Omul face urmatoarea mutare
        } 
          
        else if (whoseTurn == HUMAN) // Randul omului
        { 
            while(1)
            {
                flag = 1;
                sendString("Choose the cell: ");
                choice = scanKeypad(); // Se citeste mutarea de la tastatura sub forma de caracter
                if((choice>='1') && (choice<='9')) // Se verifica daca reprezinta o cifra de la 1 la 9
                {
                    option = choice - '0'; // Conversie la intreg
                }
                else 
                {
                    sendString("Invalid  option, please try again!"); 
                    flag = 0;  // In cazul unei mutari invalide, se reia citirea pana la alegerea unei mutari valide
                }
                switch(option) // Se cauta locul din matrice corespunzator mutarii
                {
                    case 1: {
                        x = 0; 
                        y = 0; 
                        break; 
                    }
                    case 2: {
                        x = 0; 
                        y = 1; 
                        break; 
                    }
                    case 3: {
                        x = 0; 
                        y = 2; 
                        break; 
                    }
                    case 4: {
                        x = 1; 
                        y = 0; 
                        break; 
                    }
                    case 5: {
                        x = 1; 
                        y = 1; 
                        break; 
                    }
                    case 6: {
                        x = 1; 
                        y = 2; 
                        break; 
                    }
                    case 7: {
                        x = 2; 
                        y = 0; 
                        break; 
                    }
                    case 8: {
                        x = 2; 
                        y = 1; 
                        break; 
                    }
                    case 9: {
                        x = 2; 
                        y = 2; 
                        break; 
                    }
                }
                if(flag == 1) 
                {
                    if(board[x][y] == ' ') // Daca locul este liber 
                    {
                        board[x][y] = HUMANMOVE; // Se completeaxa cu 'X'
                        break;
                    }
                    else // Daca locul corespunzator mutarii este ocupat, se reia citirea pana la alegerea unei mutari valide
                    {
                        sendString("Cell already occupied, choose another!");
                        newLine();
                    }
                }
            }
            // Se afiseaza mutarea
            sendStringWithoutNewLine("HUMAN has put ");
            transmitereSeriala(HUMANMOVE);
            sendStringWithoutNewLine(" in cell ");
            c =  option + '0';
            transmitereSeriala(c);
            showBoard(board); // Se afiseaza tabla de joc actualizata
            moveIndex ++; // Creste numarul mutarilor
            whoseTurn = COMPUTER; // Calculatorul face urmatoarea mutare
        } 
    }

  
    // Daca s-a ajuns la egal, se afiseaza mesajul corespunzator
    if (gameOver(board) == 0 &&  moveIndex == SIDE * SIDE)
    {
        sendString("It's a draw!"); 
        newLine();
    }
    else // Daca jocul a fost castigat, se afiseaza castigatorul
    { 
        if (whoseTurn == COMPUTER){
            winA++;
            if(winA == nrJocuri){
              sendString("HUMAN has won the matchs!");
              //break;
            }
             else
              sendString("HUMAN has won the game!");
        }
        else if (whoseTurn == HUMAN) {
            winB++;
            if(winB == nrJocuri){
              sendString("COMPUTER has won the matchs!");
              //break;
            }
            else
              sendString("COMPUTER has won the game!");
        }
    } 
    return; 
}  

// Functie pentru joc intre 2 persoane
void playWithFriend(int whoseTurn) 
{ 
    int moveIndex = 0, option, x, y, flag; 
    char c;
    
    // Tabla de joc este o matrice de caractere
    char board[SIDE][SIDE]; 
      
    // Initializarea jocului
    initialise(board); 
      
    // Afisarea instructiunilor
    showInstructions(); 
      
    // Jocul continua pana se ajunge la castigul unuia dintre jucatori sau egal (toata tabla este completata)
    while (gameOver(board) == 0 &&  moveIndex != SIDE*SIDE) 
    { 
        if (whoseTurn == PLAYER1) // Randul jucatorului 1
        { 
            sendString("PLAYER 1");
            while(1)
            {
                flag = 1;
                sendString("Choose the cell: ");
                choice = scanKeypad(); // Se citeste mutarea de la tastatura sub forma de caracter
                if((choice>='1') && (choice<='9')) // Se verifica daca reprezinta o cifra de la 1 la 9
                {
                    option = choice - '0'; // Conversie la intreg
                }
                else 
                {
                    sendString("Invalid  option, please try again!");
                    flag = 0; // In cazul unei mutari invalide, se reia citirea pana la alegerea unei mutari valide
                }
                switch(option) // Se cauta locul din matrice corespunzator mutarii
                {
                    case 1: {
                        x = 0; 
                        y = 0; 
                        break; 
                    }
                    case 2: {
                        x = 0; 
                        y = 1; 
                        break; 
                    }
                    case 3: {
                        x = 0; 
                        y = 2; 
                        break; 
                    }
                    case 4: {
                        x = 1; 
                        y = 0; 
                        break; 
                    }
                    case 5: {
                        x = 1; 
                        y = 1; 
                        break; 
                    }
                    case 6: {
                        x = 1; 
                        y = 2; 
                        break; 
                    }
                    case 7: {
                        x = 2; 
                        y = 0; 
                        break; 
                    }
                    case 8: {
                        x = 2; 
                        y = 1; 
                        break; 
                    }
                    case 9: {
                        x = 2; 
                        y = 2; 
                        break; 
                    }
                }
                if(flag == 1) 
                {
                    if(board[x][y] == ' ') // Daca locul este liber 
                    {
                        board[x][y] = HUMANMOVE; // Se completeaxa cu 'X'
                        break;
                    }
                    else // Daca locul corespunzator mutarii este ocupat, se reia citirea pana la alegerea unei mutari valide
                    {
                        sendString("Cell already occupied, choose another!");
                        newLine();
                    }
                }
            }
            // Se afiseaza mutarea
            sendStringWithoutNewLine("PLAYER 1 has put ");
            transmitereSeriala(HUMANMOVE);
            sendStringWithoutNewLine(" in cell ");
            c =  option + '0';
            transmitereSeriala(c);
            showBoard(board); // Se afiseaza tabla de joc actualizata
            moveIndex ++; // Creste numarul mutarilor
            whoseTurn = PLAYER2; // Celalalt jucator face urmatoarea mutare
        } 
          
        else if (whoseTurn == PLAYER2) // Randul jucatorului 1
        { 
            sendString("PLAYER 2");
            while(1)
            {
                flag = 1;
                sendString("Choose the cell: ");
                choice = scanKeypad(); // Se citeste mutarea de la tastatura sub forma de caracter
                if((choice>='1') && (choice<='9')) // Se verifica daca reprezinta o cifra de la 1 la 9
                {
                    option = choice - '0'; // Conversie la intreg
                }
                else 
                {
                    sendString("Invalid  option, please try again!");
                    flag = 0; // In cazul unei mutari invalide, se reia citirea pana la alegerea unei mutari valide
                }
                switch(option) // Se cauta locul din matrice corespunzator mutarii
                {
                    case 1: {
                        x = 0; 
                        y = 0; 
                        break; 
                    }
                    case 2: {
                        x = 0; 
                        y = 1; 
                        break; 
                    }
                    case 3: {
                        x = 0; 
                        y = 2; 
                        break; 
                    }
                    case 4: {
                        x = 1; 
                        y = 0; 
                        break; 
                    }
                    case 5: {
                        x = 1; 
                        y = 1; 
                        break; 
                    }
                    case 6: {
                        x = 1; 
                        y = 2; 
                        break; 
                    }
                    case 7: {
                        x = 2; 
                        y = 0; 
                        break; 
                    }
                    case 8: {
                        x = 2; 
                        y = 1; 
                        break; 
                    }
                    case 9: {
                        x = 2; 
                        y = 2; 
                        break; 
                    }
                }
                if(flag == 1) 
                {
                    if(board[x][y] == ' ') // Daca locul este liber 
                    {
                        board[x][y] = COMPUTERMOVE; // Se completeaxa cu '0'
                        break;
                    }
                    else // Daca locul corespunzator mutarii este ocupat, se reia citirea pana la alegerea unei mutari valide
                    {
                        sendString("Cell already occupied, choose another!");
                        newLine();
                    }
                }
            }
            // Se afiseaza mutarea
            sendStringWithoutNewLine("PLAYER 2 has put ");
            transmitereSeriala(COMPUTERMOVE);
            sendStringWithoutNewLine(" in cell ");
            c =  option + '0';
            transmitereSeriala(c);
            showBoard(board); // Se afiseaza tabla de joc actualizata
            moveIndex ++; // Creste numarul mutarilor
            whoseTurn = PLAYER1; // Celalalt jucator face urmatoarea mutare
        } 
    } 
  
    // Daca s-a ajuns la egal, se afiseaza mesajul corespunzator
    if (gameOver(board) == 0 &&  moveIndex == SIDE * SIDE)
    {
        sendString("It's a draw!"); 
        newLine();
    }
    else // Daca jocul a fost castigat, se afiseaza castigatorul
    { 
        if (whoseTurn == PLAYER1){
            winB++;
            if(winB == nrJocuri){
              sendString("PLAYER 2 has won the matchs!");
             // break;
            }
             else
               sendString("PLAYER 2 has won the game!");
        }
        else if (whoseTurn == PLAYER2){
            winA++;
            if(winA == nrJocuri){
              sendString("PLAYER 1 has won the matchs!");
             // break;
            }
             else
               sendString("PLAYER 1 has won the game!");
        }
    } 
    return; 
}                   
