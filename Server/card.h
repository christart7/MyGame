#define JOKERNUM 5
#define VALUENUM 13
#define COLORNUM 4
#define TOTALNUM (COLORNUM*VALUENUM + JOKERNUM)
#define PLAYNUM 5
class Card	                            //һ�����˿�										 
{ 
	private: 
		int m_cardarray[TOTALNUM];		 // �����˿������� 
		int m_playercard[PLAYNUM];           //��ҳ������
		void GetValue(int, char*);
		void GetColor(int, char*);
	public: 
		Card();							     // ��ʼ�����캯�� 
		void ShowCard(int n, char*);				// ���ص�n���ƵĻ�ɫ ��С 
		void Shuffle();						// ϴ��
		void Deal(char*);						// ����
		void ShowType(char*);						//��ʾ���
};	
