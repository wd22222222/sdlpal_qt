#pragma once
//#include <utility>

struct  CCellID
{
	// Attributes
public:
	int row, col;

	// Operations
public:
	CCellID(int nRow = -1, int nCol = -1) : row(nRow), col(nCol) {}

	int IsValid() const
	{
		return (row >= 0 && col >= 0);
	};
	int operator==(const CCellID& rhs) const { return (row == rhs.row && col == rhs.col); }
	int operator!=(const CCellID& rhs) const { return (operator==(rhs))==false; }
    //CCellID  CCellID::operator= (const CCellID& rhs) { col = rhs.col; row = rhs.row; return rhs; }
};

class CCellRange
{
public:

	CCellRange(int nMinRow = -1, int nMinCol = -1, int nMaxRow = -1, int nMaxCol = -1)
	{
		Set(nMinRow, nMinCol, nMaxRow, nMaxCol);
	}

	void Set(int minRow = -1, int minCol = -1, int maxRow = -1, int maxCol = -1)
	{
		m_nMinRow = minRow;
		m_nMinCol = minCol;
		m_nMaxRow = maxRow;
		m_nMaxCol = maxCol;
	};
	int  IsValid() const
	{
		return (m_nMinRow >= 0 && m_nMinCol >= 0 && m_nMaxRow >= 0 && m_nMaxCol >= 0 &&
			m_nMinRow <= m_nMaxRow && m_nMinCol <= m_nMaxCol);
	}

	int  InRange(int row, int col) const
	{
		return (row >= m_nMinRow && row <= m_nMaxRow && col >= m_nMinCol && col <= m_nMaxCol);
	}

	int  InRange(const CCellID& cellID) const
	{
		return InRange(cellID.row, cellID.col);
	}

	int  Count() { return (m_nMaxRow - m_nMinRow + 1) * (m_nMaxCol - m_nMinCol + 1); }

	CCellID  GetTopLeft() const
	{
		return CCellID(m_nMinRow, m_nMinCol);
	}

	CCellRange  Intersect(const CCellRange& rhs) const;

	int GetMinRow() const { return m_nMinRow; }
	void SetMinRow(int minRow) { m_nMinRow = minRow; }

	int GetMinCol() const { return m_nMinCol; }
	void SetMinCol(int minCol) { m_nMinCol = minCol; }

	int GetMaxRow() const { return m_nMaxRow; }
	void SetMaxRow(int maxRow) { m_nMaxRow = maxRow; }

	int GetMaxCol() const { return m_nMaxCol; }
	void SetMaxCol(int maxCol) { m_nMaxCol = maxCol; }

	int GetRowSpan() const { return m_nMaxRow - m_nMinRow + 1; }
	int GetColSpan() const { return m_nMaxCol - m_nMinCol + 1; }

	void operator=(const CCellRange& rhs)
	{
		if (this != &rhs) Set(rhs.m_nMinRow, rhs.m_nMinCol, rhs.m_nMaxRow, rhs.m_nMaxCol);
	}

	int  operator==(const CCellRange& rhs)
	{
		return ((m_nMinRow == rhs.m_nMinRow) && (m_nMinCol == rhs.m_nMinCol) &&
			(m_nMaxRow == rhs.m_nMaxRow) && (m_nMaxCol == rhs.m_nMaxCol));
	}

	int  operator!=(const CCellRange& rhs)
	{
		return (operator==(rhs))== false;
	}
	int isInRows(const int row) { return row >= m_nMinRow && row <= m_nMaxRow; };
	int isInCols(const int col) { return col >= m_nMinCol && col <= m_nMaxCol; };

protected:
	int m_nMinRow;
	int m_nMinCol;
	int m_nMaxRow;
	int m_nMaxCol;
};
