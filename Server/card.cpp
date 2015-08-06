#include <iostream>
#include <cstdlib> 
#include <ctime>
#include "card.h"

using namespace std;

Card::Card()
{
	int i;
	for(i=0;i<TOTALNUM;i++)
		m_cardarray[i] = i;
	for(i=0;i<PLAYNUM;i++)
		m_playercard[i] = i;
}

void Card::GetValue(int num, char *value)
{	
    switch(num%VALUENUM)                                  
    {   
        case 0: strcpy(value,"2");  break;   
        case 1: strcpy(value,"3");  break;   
        case 2: strcpy(value,"4");  break;   
        case 3: strcpy(value,"5");  break;   
        case 4: strcpy(value,"6");  break;   
        case 5: strcpy(value,"7");  break;    
        case 6: strcpy(value,"8");  break;   
        case 7: strcpy(value,"9");  break;   
        case 8: strcpy(value,"T");  break;   
        case 9: strcpy(value,"J");  break;   
        case 10: strcpy(value,"Q"); break;   
        case 11: strcpy(value,"K"); break;   
        case 12: strcpy(value,"A"); break;   
    }  
	
	return;
}

void Card::GetColor(int num, char *s)
{
	switch((int)num/VALUENUM)                   
    {   
        case 0: strcpy(s,"Club");  break;   
        case 1: strcpy(s,"Diamond");    break;   
        case 2: strcpy(s,"Heart");    break;   
        case 3: strcpy(s,"Spade");    break;   
    } 
	return;
}


void Card::ShowCard(int n, char *s)
{
	int num = m_playercard[n];
	char value[10];
	char color[10];
	if(num >= COLORNUM*VALUENUM)  //����
	{
		strcpy(s,"Joker");
		return;
	}
 
    GetValue(num, value);
	GetColor(num, color);
	strcpy(s, value);
	strcpy(s + strlen(value), " ");
	strcpy(s + strlen(value) + strlen(" "), color);
	return;
}

void Card::Shuffle()
{
	int tmp = 0, p1, p2;
	int n = TOTALNUM;
	int count = 1000;
	srand ( unsigned ( std::time(0) ) );

	while (--count)
	{
		p1 = rand() % n;
		p2 = rand() % n;           
		tmp = m_cardarray[p1];
		m_cardarray[p1] = m_cardarray[p2];
		m_cardarray[p2] = tmp;
	}
}

void Card::Deal(char* result)  //������Ҫ��������ţ�ʵ�ַ�ʽ����ϴһ���ƣ�ȡǰ5
{
	int i, randid;
	char tmp[64] = {'\0'};


	for (i = 0; i < PLAYNUM; i++)     
    {     
        randid = rand()%(TOTALNUM-i);  
          
        m_playercard[i] = m_cardarray[randid];

        //���ոճ鵽�������������ƽ���
        m_cardarray[randid] = m_cardarray[TOTALNUM - i-1];
		m_cardarray[TOTALNUM - i-1] = m_playercard[i];   
    }     

	int len = 0;

	for(i=0;i<PLAYNUM;i++)
	{
		ShowCard(i,tmp);
		
		strcpy(result + len,tmp);
		len += strlen(tmp);
		strcpy(result + len,"  ");
		len += 2;
	}
	result[len] = '\0';
}

//����ǰn��Ԫ�ذ�value�Ӵ�С����ͬʱcolor��Ӧ  
void sortCard(int value[], int color[], int n)        
{   
    int temp;   
    for(int i=0;i<n-1;i++)
    {   
        for(int j=i+1;j<n;j++)   
        {   
            if(value[i]<value[j])   
            {   
                temp=value[i];   
                value[i]=value[j];   
                value[j]=temp; 

				temp=color[i];   
                color[i]=color[j];   
                color[j]=temp; 
            }   
        }   
    } 

} 
#if 1
int GetTypeWithNoJoker(int v[5], int c[5])
{
	bool check = true;// 
    int i=0; 
	int tmp;
	int value[5], color[5];

	for(i=0;i<5;i++)
	{
		value[i] = v[i]; //���ܸı�ԭ���� 
		color[i] = c[i];
	}
	sortCard(value, color, 5);

	//ͬ��˳check
    for(i=0;i<5;i++)  
    {     
        if((color[0]!=color[i])||(value[0]-value[i] != i))   
        {   
            check = false;
			break;   
        }   
    }
	if(check == true){
		return 256;
	}

	//����check
    if(value[0]==value[3] || value[1]==value[4]) 
	{
        return 128;
	}

	//��«check
	if((value[2]==value[0] && value[3]==value[4])
		||(value[2]==value[4] && value[0]==value[1]))
	{
		return 64; 
	}

	//ͬ��check
	check = true;
    i=0;
    while(i<5)   
    {   
        if(color[0]!=color[i])   
        {   
            check=false;
			break;   
        }   
        i++;   
    }
	if(check == true){
		return 32;
	}

	//˳��check
	check = true;
    i=0;
    while(i<5)   
    {   
        if(value[0]-value[i] != i)   
        {   
            check=false;
			break;   
        }   
        i++;   
    }
	if(check == true)
	{
		return 16;
	}

	//����check
	tmp = 0;
	for(i=0;i<5;i++)   
    {   
		if(value[2]==value[i]){   
            tmp++; 
		}
    }   
    if(tmp==3)// && (v[0]+v[4])!=(v[1]+v[3]))   
        return 8;

	//����
	tmp = 0;
	for(i=0;i<4;i++)   
    {   
        if(value[i]==value[i+1])   
        {   
            i++;   
            tmp++;   
        }   
    }

    if(tmp==2 && (value[1]!=value[3]))	
	{
        return 4; 
	}

	//����
	if(tmp==1){
		return 2;
	}
	
	//ɢ��
	return 0;
}


void Card::ShowType(char* type)
{
	int jokernum = 0;
	int value[5];
	int color[5];
	int tmp=0;
	int i,res;

	for(i=0;i<PLAYNUM;i++)
	{
		if(m_playercard[i]>=COLORNUM*VALUENUM)
		{	
			jokernum++;
		}
		else
		{
			value[tmp] = m_playercard[i]%VALUENUM;
			color[tmp] = m_playercard[i]/VALUENUM;
			tmp++;
		}
	}

	//�Ӵ�С���м��
	if(jokernum == 5)//���
	{
		strcpy(type, "Five Of Jokers");
		return;
	}

	if(jokernum == 4)//�ض�Ϊ5��
	{
		strcpy(type, "Five Of Kind ");
		GetValue(value[0], type + strlen("Five Of Kind "));
		return;
	}

	if(jokernum == 3)// ����Ϊ���� ͬ��˳ ����
	{
		sortCard(value, color, 2);

		if(value[1] == value[0])//����
		{
			strcpy(type, "Five Of Kind ");
			GetValue(value[0], type + strlen("Five Of Kind "));
		}
		else if((color[1] == color[0])&&(value[0] - value[0]<5))//ͬ��˳
		{
			strcpy(type, "Straight Flush ");
		}
		else					//����
		{
			strcpy(type, "Four Of Kind ");
			GetValue(value[0], type + strlen("Four Of Kind "));
		}
		return;
	}

	if(jokernum == 2)
	{
		sortCard(value, color, 3);

		if(value[0] == value[2])//����
		{
			strcpy(type, "Five Of Kind ");
			GetValue(value[0], type + strlen("Five Of Kind "));
			return;
		}
			
		if(color[0]==color[1] && color[0]==color[2]  //ͬ��˳ 
		&& value[0]!=value[1] && value[1]!=value[2]
		&& value[0]-value[2]<5)
		{
			strcpy(type, "Straight Flush");
			return;
		}

		if(value[0]==value[1] || value[1]==value[2])//����
		{
			strcpy(type, "Four Of Kind ");
			GetValue(value[1], type + strlen("Four Of Kind "));
			return;
		}

		if(color[0]==color[1] && color[0]==color[2]) //ͬ��
		{
			strcpy(type, "Flush");
			return;
		}

		if(value[0]!=value[1] && value[1]!=value[2] && value[0]-value[2]<5) //˳��
		{
			strcpy(type, "Straight");
			return;
		}

		strcpy(type, "Three Of Kind ");
		GetValue(value[0], type + strlen("Three Of Kind "));
		return;
	}

	if(jokernum == 1)	
	{
		if(value[0] == value[1] && value[2]==value[3] && value[1] == value[2])//����
		{
			strcpy(type, "Five Of Kind ");
			GetValue(value[0], type + strlen("Five Of Kind "));
			return;
		}
		else
		{
			res = 0;
			for(i=0;i<COLORNUM*VALUENUM;i++)
			{
				//�Ʋ����ظ�
				if((i==m_playercard[0])||(i==m_playercard[1])
				 ||(i==m_playercard[2])||(i==m_playercard[3]))
				{
					continue;
				}

				value[4] = i%VALUENUM;
				color[4] = i/VALUENUM;

				tmp = GetTypeWithNoJoker(value, color);
				if(tmp > res){
					res = tmp;
				}
			}
		}
	}
	else
	{
		res = GetTypeWithNoJoker(value, color);	
	}
	
	switch (res)
	{
		case 256:
			strcpy(type, "Straight Flush"); break;
		case 128:
			strcpy(type, "Four Of Kind "); break;
		case 64:
			strcpy(type, "Full House"); break;
		case 32:
			strcpy(type, "Flush"); break;
		case 16:
			strcpy(type, "Straight"); break;
		case 8:
			strcpy(type, "Three Of Kind"); break;
		case 4:
			strcpy(type, "Two Pair"); break;
		case 2:
			strcpy(type, "One Pair"); break;
		default:
			strcpy(type, "Odd Card"); break;
	}

		return;
}
	
#endif
