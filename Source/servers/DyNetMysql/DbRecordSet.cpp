#include "DbRecordSet.h"
#include "DbRecord.h"

const uint32 INVALID_KEY = UINT_MAX;

DbRecordSet::DbRecordSet():m_nCursor( 0 ),m_nKeyIndex( INVALID_KEY ) ,m_nAutoIncIndex( INVALID_KEY )
{

}

DbRecordSet::~DbRecordSet()
{
	while (m_vecRecord.size() != 0) 
	{
		DbRecord* pRec = m_vecRecord[m_vecRecord.size() - 1];
		S_SAFE_RELEASE( pRec );
	}

	m_nCursor = 0;
	m_nKeyIndex = INVALID_KEY;
	m_vecFieldInfo.clear();
	m_vecRecord.clear();
}

void DbRecordSet::Release()
{
	delete this;
}

vector<DbFieldInfo>& DbRecordSet::GetFieldInfos()
{
	return m_vecFieldInfo;
}

int32 DbRecordSet::FieldsCount() const
{
	return m_vecFieldInfo.size();
}

char* DbRecordSet::GetTableName()
{
	return m_arrTableName;
}

const DbFieldInfo&	DbRecordSet::GetFieldInfo(int32 nIndex) const
{
	return m_vecFieldInfo[nIndex];
}

int32 DbRecordSet::Rows() const
{
	return m_vecRecord.size();
}

// 将查询到的数据源(多条), 
bool DbRecordSet::Create( MYSQL_RES* pRes )
{
	if (!pRes) return false;

	// 填写字段信息
	MYSQL_FIELD* field = NULL;
	uint32 unNumFields = mysql_num_fields( pRes );
	MYSQL_FIELD* fields = mysql_fetch_fields( pRes );
	for (uint32 i = 0 ; i < unNumFields; i++)
	{
		field = mysql_fetch_field_direct(pRes,i);
		DbFieldInfo fieldInfo;
		fieldInfo.strName = field->name;
		fieldInfo.nType = field->type;
		fieldInfo.nAttr = field->flags;
		fieldInfo.nLen = field->length;
		m_vecFieldInfo.push_back( fieldInfo );
		if (field->flags & PRI_KEY_FLAG )
			m_nKeyIndex = i;
		if ( field->flags & AUTO_INCREMENT_FLAG )
			m_nAutoIncIndex = i;
	}

	if ( !fields )
	{
		return false;
	}

	int64 ulNumRecord = mysql_num_rows( pRes );
	for ( uint32 r = 0 ; r < ulNumRecord ; r++ )
	{
		mysql_data_seek( pRes , r );// 移动到第i个记录 
		MYSQL_ROW row = mysql_fetch_row( pRes );// 取第i个记录数据 
		if ( !row ) continue;
		DbRecord* pRec = new DbRecord( *this , row , unNumFields );
		m_vecRecord.push_back( pRec );
	}

	this->Move(0);
	strcpy( m_arrTableName , field->table );
	return true;
}


void DbRecordSet::Remove( DbRecord* pRec )
{	
	std::vector< DbRecord* >::iterator itor = std::find(m_vecRecord.begin(),m_vecRecord.end(),pRec);
	if ( itor != m_vecRecord.end() )
	{
		m_vecRecord.erase(itor);
	}
}

IDbRecord* DbRecordSet::GetRecord() const
{	
	if ( 0 > m_nCursor )
		return NULL;
	return m_vecRecord[m_nCursor];
}

void* DbRecordSet::GetRecordData(uint32 nIndex) const 
{	
	if ( nIndex >= m_vecRecord.size() )
		return NULL;
	return (m_vecRecord[nIndex])->GetRowBuff();
}

void DbRecordSet::Move( uint32 nIndex )
{	
	if ( m_vecRecord.empty() )
	{
		m_nCursor = -1;
		return;
	}
	if ( nIndex >= m_vecRecord.size() )
		m_nCursor = (int32)m_vecRecord.size() - 1;
	else
		m_nCursor = nIndex;
}


