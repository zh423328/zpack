#ifndef __ZP_FILE_H__
#define __ZP_FILE_H__

#include "zpack.h"

namespace zp
{

class Package;

//�ļ�δ����ѹ����ȡ
class File : public IReadFile
{
public:
	File(const Package* package, u64 offset, u32 size, u32 flag, u64 nameHash);
	~File();

	virtual u32 size() const;

	virtual u32 availableSize() const;

	virtual u32 flag() const;

	virtual void seek(u32 pos);

	virtual u32 tell() const;

	virtual u32 read(u8* buffer, u32 size);

private:
	void seekInPackage();

private:
	u64				m_offset;		//ƫ����
	u64				m_nameHash;		//hash
	const Package*	m_package;		//zpk��ָ��
	u32				m_flag;			//flag
	u32				m_size;			//�ļ���С
	u32				m_readPos;		//��ȡλ��
};

}

#endif
