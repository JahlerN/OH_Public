#include "stdafx.h"

QueryResult::QueryResult(MYSQL_RES* result, MYSQL_FIELD* fields, uint64 rowCount, uint32 fieldCount)
{
	m_result = result;
	m_fields = fields;
	m_rowCount = rowCount;
	m_fieldCount = fieldCount;

	NextRow();
}

QueryResult::~QueryResult()
{
	Delete();
}

void QueryResult::Delete()
{
	if (m_result != NULL)
	{
		mysql_free_result(m_result);
		m_result = NULL;
	}

	/*if (m_lengths != NULL)
	{
		delete m_lengths;
		m_lengths = NULL;
	}*/
	
}

bool QueryResult::NextRow()
{
	if (m_result == NULL)
		return false;

	m_curRow = mysql_fetch_row(m_result);

	//If it's null, end of fetch.
	if (m_curRow == NULL)
	{
		Delete();
		return false;
	}

	m_lengths = mysql_fetch_lengths(m_result);
	if (m_lengths == NULL)
	{
		Delete();
		return false;
	}

	m_rowIndex = 0;
	return true;
}

bool QueryResult::GetBool()
{
	if (m_fields[m_rowIndex].type != MYSQL_TYPE_TINY)
	{
		printf("\nIncorrect value type: %s.%s, Actual type: %d\n", m_fields[m_rowIndex].org_table, m_fields[m_rowIndex].org_name, m_fields[m_rowIndex].type);
		m_rowIndex++;
		return 0;
	}
	
	return static_cast<int8>(strtol((char*)m_curRow[m_rowIndex++], NULL, 10)) != 1;
}

int8 QueryResult::GetInt8()
{
	if (m_fields[m_rowIndex].type != MYSQL_TYPE_TINY)
	{
		printf("\nIncorrect value type: %s.%s, Actual type: %d\n", m_fields[m_rowIndex].org_table, m_fields[m_rowIndex].org_name, m_fields[m_rowIndex].type);
		m_rowIndex++;
		return 0;
	}

	return static_cast<int8>(strtol((char*)m_curRow[m_rowIndex++], NULL, 10));
}

uint8 QueryResult::GetUint8()
{
	if (m_fields[m_rowIndex].type != MYSQL_TYPE_TINY)
	{
		printf("\nIncorrect value type: %s.%s, Actual type: %d\n", m_fields[m_rowIndex].org_table, m_fields[m_rowIndex].org_name, m_fields[m_rowIndex].type);
		m_rowIndex++;
		return 0;
	}

	return static_cast<uint8>(strtoul((char*)m_curRow[m_rowIndex++], NULL, 10));
}

int16 QueryResult::GetInt16()
{
	if (m_fields[m_rowIndex].type != MYSQL_TYPE_SHORT)
	{
		printf("\nIncorrect value type: %s.%s, Actual type: %d\n", m_fields[m_rowIndex].org_table, m_fields[m_rowIndex].org_name, m_fields[m_rowIndex].type);
		m_rowIndex++;
		return 0;
	}

	return static_cast<int16>(strtol((char*)m_curRow[m_rowIndex++], NULL, 10));
}

uint16 QueryResult::GetUint16()
{
	if (m_fields[m_rowIndex].type != MYSQL_TYPE_SHORT)
	{
		printf("\nIncorrect value type: %s.%s, Actual type: %d\n", m_fields[m_rowIndex].org_table, m_fields[m_rowIndex].org_name, m_fields[m_rowIndex].type);
		m_rowIndex++;
		return 0;
	}

	return static_cast<uint16>(strtoul((char*)m_curRow[m_rowIndex++], NULL, 10));
}

int32 QueryResult::GetInt32()
{
	if (m_fields[m_rowIndex].type != MYSQL_TYPE_LONG && m_fields[m_rowIndex].type != MYSQL_TYPE_INT24)
	{
		printf("\nIncorrect value type: %s.%s, Actual type: %d\n", m_fields[m_rowIndex].org_table, m_fields[m_rowIndex].org_name, m_fields[m_rowIndex].type);
		m_rowIndex++;
		return 0;
	}

	return static_cast<int32>(strtol((char*)m_curRow[m_rowIndex++], NULL, 10));
}

uint32 QueryResult::GetUint32()
{
	if (m_fields[m_rowIndex].type != MYSQL_TYPE_LONG && m_fields[m_rowIndex].type != MYSQL_TYPE_INT24)
	{
		printf("\nIncorrect value type: %s.%s, Actual type: %d\n", m_fields[m_rowIndex].org_table, m_fields[m_rowIndex].org_name, m_fields[m_rowIndex].type);
		m_rowIndex++;
		return 0;
	}

	return static_cast<uint32>(strtoul((char*)m_curRow[m_rowIndex++], NULL, 10));
}

int64 QueryResult::GetInt64()
{
	if (m_fields[m_rowIndex].type != MYSQL_TYPE_LONGLONG)
	{
		printf("\nIncorrect value type: %s.%s, Actual type: %d\n", m_fields[m_rowIndex].org_table, m_fields[m_rowIndex].org_name, m_fields[m_rowIndex].type);
		m_rowIndex++;
		return 0;
	}

	return static_cast<int64>(strtoll((char*)m_curRow[m_rowIndex++], NULL, 10));
}

uint64 QueryResult::GetUint64()
{
	if (m_fields[m_rowIndex].type != MYSQL_TYPE_LONGLONG)
	{
		printf("\nIncorrect value type: %s.%s, Actual type: %d\n", m_fields[m_rowIndex].org_table, m_fields[m_rowIndex].org_name, m_fields[m_rowIndex].type);
		m_rowIndex++;
		return 0;
	}

	return static_cast<uint64>(strtoull((char*)m_curRow[m_rowIndex++], NULL, 10));
}

float QueryResult::GetFloat()
{
	if (m_fields[m_rowIndex].type != MYSQL_TYPE_FLOAT)
	{
		printf("\nIncorrect value type: %s.%s, Actual type: %d\n", m_fields[m_rowIndex].org_table, m_fields[m_rowIndex].org_name, m_fields[m_rowIndex].type);
		m_rowIndex++;
		return 0.0f;
	}

	return static_cast<float>(atof((char*)m_curRow[m_rowIndex++]));
}

double QueryResult::GetDouble()
{
	if (m_fields[m_rowIndex].type != MYSQL_TYPE_DOUBLE)
	{
		printf("\nIncorrect value type: %s.%s, Actual type: %d\n", m_fields[m_rowIndex].org_table, m_fields[m_rowIndex].org_name, m_fields[m_rowIndex].type);
		m_rowIndex++;
		return 0.0;
	}

	return static_cast<double>(atof((char*)m_curRow[m_rowIndex++]));
}

char const* QueryResult::GetCString()
{
	if (m_curRow[m_rowIndex] == NULL)
	{
		m_rowIndex++;
		return NULL;
	}

	if (m_fields[m_rowIndex].type == MYSQL_TYPE_TINY ||
		m_fields[m_rowIndex].type == MYSQL_TYPE_SHORT ||
		m_fields[m_rowIndex].type == MYSQL_TYPE_INT24 ||
		m_fields[m_rowIndex].type == MYSQL_TYPE_LONG ||
		m_fields[m_rowIndex].type == MYSQL_TYPE_FLOAT ||
		m_fields[m_rowIndex].type == MYSQL_TYPE_DOUBLE ||
		m_fields[m_rowIndex].type == MYSQL_TYPE_LONGLONG)
	{
		m_rowIndex++;
		return NULL;
	}

	return static_cast<char const*>(m_curRow[m_rowIndex++]);
}

std::string QueryResult::GetString()
{
	if (m_curRow[m_rowIndex] == NULL)
	{
		m_rowIndex++;
		return "";
	}

	char const* temp = GetCString();
	if (temp == NULL)
		return "";

	return std::string(temp);
}

ByteBuffer QueryResult::GetBinary()
{
	ByteBuffer ret;
	if (m_curRow[m_rowIndex] == NULL || m_lengths[m_rowIndex] == NULL)
	{
		m_rowIndex++;
		return ret;
	}

	ret.resize(m_lengths[m_rowIndex]);
	ret.append(m_curRow[m_rowIndex], m_lengths[m_rowIndex]);
	m_rowIndex++;
	return ret;
}
