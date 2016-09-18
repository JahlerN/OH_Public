#ifndef QUERYRESULT_H
#define QUERYRESULT_H

#include <mysql.h>

class QueryResult
{
public:
	QueryResult(MYSQL_RES* result, MYSQL_FIELD* fields, uint64 rowCount, uint32 fieldCount);
	~QueryResult();

	void Delete();

	bool IsEmpty();
	bool NextRow();

	bool GetBool();
	int8 GetInt8();
	uint8 GetUint8();
	int16 GetInt16();
	uint16 GetUint16();
	int32 GetInt32();
	uint32 GetUint32();
	int64 GetInt64();
	uint64 GetUint64();
	float GetFloat();
	double GetDouble();
	char const* GetCString();
	std::string GetString();
	//This is for blob type fields.
	ByteBuffer GetBinary();


private:
	uint64 m_rowCount;
	uint32 m_fieldCount;
	MYSQL_RES* m_result;
	MYSQL_FIELD* m_fields;

	MYSQL_ROW m_curRow;
	unsigned long* m_lengths;
	uint32 m_rowIndex = 0;
};

#endif