#define JOKERNUM 5
#define VALUENUM 13
#define COLORNUM 4
#define TOTALNUM (COLORNUM*VALUENUM + JOKERNUM)
#define PLAYNUM 5
class Card	                            //一整副扑克										 
{ 
	private: 
		int m_cardarray[TOTALNUM];		 // 定义扑克类数组 
		int m_playercard[PLAYNUM];           //玩家抽出的牌
		void GetValue(int, char*);
		void GetColor(int, char*);
	public: 
		Card();							     // 初始化构造函数 
		void ShowCard(int n, char*);				// 返回第n张牌的花色 大小 
		void Shuffle();						// 洗牌
		void Deal(char*);						// 发牌
		void ShowType(char*);						//显示结果
};	
