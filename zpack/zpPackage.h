#ifndef __ZP_PACKAGE_H__
#define __ZP_PACKAGE_H__

#include "zpack.h"
#include <string>
#include <vector>
#include "stdio.h"

#include "pthread.h"

namespace zp
{

#if defined (ZP_USE_WCHAR)
#define Remove _wremove
#define Rename _wrename
	typedef std::wistringstream IStringStream;
#else
#define Remove remove
#define Rename rename
	typedef std::istringstream IStringStream;
#endif

	// fix: modify PACKAGE_FILE_SIGN to DXMP
	const u32 PACKAGE_FILE_SIGN = 'DXMP';		 //��ǣ���ӽ���ͷ
	const u32 CURRENT_VERSION = '0030';			 //�汾��

	///////////////////////////////////////////////////////////////////////////////////////////////////
	//zpk��ͷ
	struct PackageHeader
	{
		u32	sign;						//���
		u32	version;					//�汾��
		u32	headerSize;					//��ͷ��С
		u32	fileCount;					//�ļ�����
		u64	fileEntryOffset;			//�ļ�ʵ�忪ʼ��ȡ��λ�ã�ѹ�����ģ����ļ�ʵ���ȡ���ļ�ǰ��
		u64 filenameOffset;				//�ļ�����ʼ��ȡѹ����λ�ã��ļ�����ȡ�����
		u32	allFileEntrySize;			//ѹ�������ļ�ʵ���ֽ����ܳ��ȣ�compress��
		u32 allFilenameSize;			//ѹ�����ļ����ܳ���
		u32 originFilenamesSize;	//filename size before compression δѹ�������ļ����ܳ���
		u32 chunkSize;				//file compress unit Ĭ�Ͽ��СΪ256k
		u32	flag;					//��׼ ��unicode���Ƕ��ֽڣ�Ĭ��Ϊ0���ֽ�
		u32 fileEntrySize;			//Ĭ��Ϊÿ��ʵ��Ĵ�С
		u32 reserved[18];			//Ԥ�����ݣ���ʱû��
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////
	//ÿ���ļ����
	struct FileEntry
	{
		u64	byteOffset;  //ÿ���ļ����ݵ�ƫ��λ��
		u64	nameHash;	//ÿ���ļ�����hash
		u32	packSize;	//�ļ�����ѹ����Ĵ�С�����ļ���Сsize in package(may be compressed)
		u32 originSize;	//�ļ�ԭʼ��С
		u32 flag;		//zp::FILE_COMPRESS ��ʾ�ļ�����ѹ�� zp::FILE_DELETE ��ʾ�ļ�ɾ��
		u32 chunkSize;	//can be different with chunkSize in package header����ʱ���ڰ�ͷ��chunsize����ʲô��;����
		u64 contentHash; //Ĭ��Ϊ0���������ݲ���
		u32 availableSize;	//һ��Ϊ�ļ���С
		u32 reserved;		//��ʱû����;��Ĭ��Ϊ0
	};

	//����ȡ
	///////////////////////////////////////////////////////////////////////////////////////////////////
	class Package : public IPackage
	{
		friend class File;
		friend class CompressedFile;
		friend class WriteFile;

	public:
		Package(const Char* filename, bool readonly, bool readFilename);
		~Package();

		//�Ƿ���Ч
		bool valid() const;

		//�Ƿ�ֻ��
		virtual bool readonly() const;

		//����
		virtual const Char* packageFilename() const;

		//�Ƿ����ļ���
		virtual bool hasFile(const Char* filename) const;

		//���ļ�
		virtual IReadFile* openFile(const Char* filename);
		//�ر��ļ�
		virtual void closeFile(IReadFile* file);

		//�ļ�����
		virtual u32 getFileCount() const;
		virtual bool getFileInfo(u32 index, Char* filenameBuffer, u32 filenameBufferSize, u32* fileSize = 0,
			u32* packSize = 0, u32* flag = 0, u32* availableSize = 0, u64* contentHash = 0) const;
		virtual bool getFileInfo(const Char* filename, u32* fileSize = 0, u32* packSize = 0,
			u32* flag = 0, u32* availableSize = 0, u64* contentHash = 0) const;

		//����ļ�
		virtual bool addFile(const Char* filename, const Char* exterFilename, u32 fileSize, u32 flag,
			u32* outPackSize = 0, u32* outFlag = 0, u32 chunkSize = 0);
		//����һ���ļ�
		virtual IWriteFile* createFile(const Char* filename, u32 fileSize, u32 packSize,
			u32 chunkSize = 0, u32 flag = 0, u64 contentHash = 0);
		virtual IWriteFile* openFileToWrite(const Char* filename);
		virtual void closeFile(IWriteFile* file);

		//�Ƴ��ļ�
		virtual bool removeFile(const Char* filename);
		virtual bool dirty() const;
		virtual void flush();

		//
		virtual bool defrag(Callback callback, void* callbackParam);

		//��ʱûʲô��
		virtual u32 getFileUserDataSize() const;
		virtual bool writeFileUserData(const Char* filename, const u8* data, u32 dataLen);
		virtual bool readFileUserData(const Char* filename, u8* data, u32 dataLen);

	private:
		//��ȡ�ļ�ͷ
		bool readHeader();
		//��ȡ�ļ�fileentries
		bool readFileEntries();
		//��ȡ�ļ�����
		bool readFilenames();

		//�Ƴ���Ҫɾ����fileʵ��
		void removeDeletedEntries();
		void writeTables(bool avoidOverwrite);

		//����hash��
		bool buildHashTable();


		int getFileIndex(const Char* filename) const;
		int getFileIndex(u64 nameHash) const;
		u32 insertFileEntry(FileEntry& entry, const Char* filename);
		bool insertFileHash(u64 nameHash, u32 entryIndex);

		//ͨ��str��һ�����ļ�·����
		u64 stringHash(const Char* str, u32 seed) const;

		void fixHashTable(u32 index);

		//д��zpk
		void writeRawFile(FileEntry& entry, FILE* file);

		//for writing file
		u32 getFileAvailableSize(u64 nameHash) const;
		bool setFileAvailableSize(u64 nameHash, u32 size);

		FileEntry& getFileEntry(u32 index) const;

	private:

		mutable pthread_mutex_t mutex_;

		String					m_packageFilename; //zpk����
		mutable FILE*			m_stream;		//�򿪵�zpk��ָ��
		PackageHeader			m_header;		//��ͷ
		u32						m_hashBits;		//hashtable ��Ӧ��λ������СΪ8--��Ӧhash������Ϊ2^8-1 =256��
		std::vector<int>		m_hashTable;	//hashtable,ʵ������hashtble��index = ʵ��.namehash&m_hashMask��
		std::vector<u8>			m_fileEntries;	//�ļ�fileEntry������
		std::vector<String>		m_filenames;	//�ļ�������
		u64						m_packageEnd;	//��ǰ����β��ƫ��λ��
		u32						m_hashMask;		//���� m_hashMask = (1 << m_hashBits) - 1;
		std::vector<u8>			m_chunkData;
		std::vector<u8>			m_compressBuffer;
		std::vector<u32>		m_chunkPosBuffer;
		mutable void*			m_lastSeekFile;
		bool					m_readonly;
		bool					m_dirty;
	};

	//��ȡ�ļ�����
	///////////////////////////////////////////////////////////////////////////////////////////////////
	inline u32 Package::getFileCount() const
	{
		//m_header.fileEntrySize ÿ���ļ�ʵ��Ĵ�С
		return (u32)(m_fileEntries.size() / m_header.fileEntrySize);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	//��ȡ������ļ�ʵ��
	inline FileEntry& Package::getFileEntry(u32 index) const
	{
		return *((FileEntry*)&m_fileEntries[index * m_header.fileEntrySize]);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	class Lock
	{
	public:
		Lock(pthread_mutex_t* mutex) : mutex_(mutex){ pthread_mutex_lock(mutex_); }
		~Lock()	{ pthread_mutex_unlock(mutex_); }
		pthread_mutex_t* mutex_;
	};

#define SCOPE_LOCK		Lock localLock(&mutex_)
#define PACKAGE_LOCK	Lock localLock(&(m_package->mutex_))
};
#endif