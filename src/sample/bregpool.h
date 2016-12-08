/*
 *	bregpool.h
 */


class BregPool
{
public:
	BregPool(int max){
		m_nmax = max;
		m_rxpool = new BREGEXP*[m_nmax]; 
	//	ZeroMemory(m_rxpool,sizeof(BREGEXP*)*m_nmax);
		memset(m_rxpool,0,sizeof(BREGEXP*)*m_nmax);
	};
	~BregPool() {
		Free();
	};
	void Free() {
		if (m_rxpool == 0)
			return;
		for (int i = 0;i < m_nmax;i++) {
			if (m_rxpool[i])
				BRegfree(m_rxpool[i]);
		}
		delete [] m_rxpool;
		m_rxpool = NULL;
	};
	BREGEXP* Get(TCHAR *regstr)
	{
		BREGEXP *r;
		int i;
		for (i = 0;i < m_nmax;i++) {
			r = m_rxpool[i];
			if (r == 0)
				break;
			if (r->parap == 0)
				break;
			// Check same Regular Expression
			if (memcmp(regstr,r->parap,(r->paraendp - r->parap) + 1) == 0)
				return r;		// we got !!!
		}
		if (i > m_nmax - 1)
			i = m_nmax - 1;
		if (m_rxpool[i])
			return m_rxpool[i];
		TCHAR msg[BREGEXP_MAX_ERROR_MESSAGE_LEN];
		TCHAR p[] = _T(" ");
		// Make Compile Block
		BMatch(regstr,p,p+1,&m_rxpool[i],msg);
		
		return m_rxpool[i];
	}
private:
	int m_nmax;
	BREGEXP **m_rxpool;
};

