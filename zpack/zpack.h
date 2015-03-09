#ifndef __ZPACK_H__
#define __ZPACK_H__

#include <string>

// #if defined (_MSC_VER) && defined (UNICODE)
// 	#define ZP_USE_WCHAR
// #endif

//#if defined (_MSC_VER)
#define ZP_CASE_SENSITIVE	0
//#else
//	#define ZP_CASE_SENSITIVE	1
//#endif

namespace zp
{
#if defined (ZP_USE_WCHAR)
	typedef wchar_t Char;
	#ifndef _T
		#define _T(str) L##str
	#endif
	typedef std::wstring String;
	#define Fopen _wfopen
	#define Strcpy wcscpy_s
#else
	typedef char Char;
	#ifndef _T
		#define _T(str) str
	#endif
	typedef std::string String;
	#define Fopen fopen
	#define Strcpy strcpy_s
#endif

//��ʵ�⹦����Ҫ�Ǳ�֤64λ�����룬��С���䡣��
#ifndef __LP64__
	typedef unsigned char u8;					//1���ֽ�
	typedef unsigned short u16;					//2���ֽ�
	typedef unsigned long u32;					//4���ֽ�
	typedef unsigned long long u64;				//8���ֽ�
#else
	//64λ�仯����long��ָ���С
	typedef unsigned char u8;					//1���ֽ�
	typedef unsigned short u16;					//2���ֽ�
	typedef unsigned int u32;					//4���ֽ�
	typedef unsigned long long u64;				//8���ֽڣ�64λϵͳ��С���䣬32λϵͳҲ��8�ֽڣ�
#endif


const u32 MAX_FILENAME_LEN = 1024;

//���ļ�ģʽ
const u32 OPEN_READONLY = 1;
const u32 OPEN_NO_FILENAME = 2;

const u32 PACK_UNICODE = 1;

//�ļ�ģʽflag
const u32 FILE_DELETE = (1<<0);			//ɾ��
const u32 FILE_COMPRESS = (1<<1);		//ѹ��
//const u32 FILE_WRITING = (1<<2);

const u32 FILE_FLAG_USER0 = (1<<10);	
const u32 FILE_FLAG_USER1 = (1<<11);

typedef bool (*Callback)(const Char* path, zp::u32 fileSize, void* param);

class IReadFile;
class IWriteFile;

///////////////////////////////////////////////////////////////////////////////////////////////////
//zpk���ӿ�
class IPackage
{
public:
	//�ļ��Ƿ�ֻ��
	virtual bool readonly() const = 0;
	//������
	virtual const Char* packageFilename() const = 0;

	///////////////////////////////////////////////////////////////////////////////////////////////
	//readonly functions, not available when package is dirty
	//IFile will become unavailable after package is modified
	//�Ƿ���ĳ���ļ���
	virtual bool hasFile(const Char* filename) const = 0;

	//���ļ�����ʵ�Ǵ�.zpk���ҵ���Ӧ�����ݿ�
	virtual IReadFile* openFile(const Char* filename) = 0;
	//�ر�ĳ���ļ�
	virtual void closeFile(IReadFile* file) = 0;

	//��ȡ�м��ļ�������
	virtual u32 getFileCount() const = 0;

	//��ȡĳ���ļ���Ϣ��index/filename��
	virtual bool getFileInfo(u32 index, Char* filenameBuffer, u32 filenameBufferSize, u32* fileSize = 0,
							u32* packSize = 0, u32* flag = 0, u32* availableSize = 0, u64* contentHash = 0) const = 0;
	virtual bool getFileInfo(const Char* filename, u32* fileSize = 0, u32* packSize = 0,
							u32* flag = 0, u32* availableSize = 0, u64* contentHash = 0) const = 0;

	///////////////////////////////////////////////////////////////////////////////////////////////
	//package manipulation fuctions, not available in read only mode

	//do not add same file more than once between flush() call
	//outFileSize	origin file size
	//outPackSize	size in package

	//���һ���ļ���zpk��
	virtual bool addFile(const Char* filename, const Char* externalFilename, u32 fileSize, u32 flag,
						u32* outPackSize = 0, u32* outFlag = 0, u32 chunkSize = 0) = 0;
	
	//����һ���ļ�
	virtual IWriteFile* createFile(const Char* filename, u32 fileSize, u32 packSize,
									u32 chunkSize = 0, u32 flag = 0, u64 contentHash = 0) = 0;

	//��һ���ļ�д��
	virtual IWriteFile* openFileToWrite(const Char* filename) = 0;
	virtual void closeFile(IWriteFile* file) = 0;

	//can not remove files added after last flush() call
	//ɾ��һ���ļ�
	virtual bool removeFile(const Char* filename) = 0;

	//return true if there's any unsaved change of package
	virtual bool dirty() const = 0;

	//package file won't change before calling this function
	//����ˢ�½�����
	virtual void flush() = 0;

	//��ʱ�������ʲô��
	virtual bool defrag(Callback callback, void* callbackParam) = 0;	//can be very slow, don't call this all the time

	virtual u32 getFileUserDataSize() const = 0;

	virtual bool writeFileUserData(const Char* filename, const u8* data, u32 dataLen) = 0;
	virtual bool readFileUserData(const Char* filename, u8* data, u32 dataLen) = 0;

protected:
	virtual ~IPackage(){}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//��ȡ�ļ��ӿڣ�����FILE�ӿ�
class IReadFile
{
public:
	virtual u32 size() const = 0;

	virtual u32 availableSize() const = 0;

	virtual u32 flag() const = 0;

	virtual void seek(u32 pos) = 0;

	virtual u32 tell() const = 0;

	//��ȡsize���ݣ����buffer��
	virtual u32 read(u8* buffer, u32 size) = 0;

protected:
	virtual ~IReadFile(){}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//���ļ�����zpk����
class IWriteFile
{
public:
	virtual u32 size() const = 0;

	virtual u32 flag() const = 0;

	virtual void seek(u32 pos) = 0;

	virtual u32 tell() const = 0;

	virtual u32 write(const u8* buffer, u32 size) = 0;

protected:
	virtual ~IWriteFile(){}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//����һ��zpk��
IPackage* create(const Char* filename, u32 chunkSize = 0x40000, u32 fileUserDataSize = 0); //2^18 = 256k

//��һ��zpk��
IPackage* open(const Char* filename, u32 flag = OPEN_READONLY | OPEN_NO_FILENAME);

//�ر�res.zpk��
void close(IPackage* package);

}

#endif
