////////////////////////////////////////////////////////////////////////////////////////////////
// �V���A���ʐM�x���N���X Ver 0.81 
// cserial.cpp ����
// 2017.12.15 Fri.
// �c�� ����
// 0.1 �����J (2004.5.28 Fri.)
// 0.11 ~CSerial���\�b�h�ɊԈႢ���������̂ŏC�� (2004.5.31 Mon.)
// 0.12 ReceiveSerialData��MakeBuffer���\�b�h�ɊԈႢ���������̂ŏC��
// 0.20 FIFO�������̗ʂ����邱�Ƃ��\�ɂȂ�B(2006.6.9 Fri.)
// 0.3 COM10�ȍ~���T�|�[�g(2008.5.15 Thr.)
// 0.4 ���b�Z�[�W�{�b�N�X��\�����Ȃ������\�ɂ���(�w�b�_�t�@�C���Őݒ�\)�B(2009.5.18 Mon.)
// 0.5 UNICODE �Ή�(2011.5.18 Wed.)
// 0.6 COM�|�[�g�����@�\��ǉ��Aprintf�n�̊֐���pintf_s�ɕύX(2014.9.15 Mon.)
// 0.7 sprintf�n�̊֐���sprintf_s�݂̂ɂ����̂�I���ł���悤�ɕύX�L���ɂ���ɂ�cserial.h��(33�s)
//     #define __NOT_USE_SPRINTF_S
//     ���R�����g�A�E�g���Ă���̂ł���̃R�����g�A�E�g����������΂悢�B
//     �Ō�ɊJ�����V���A���|�[�g��COM�ԍ���ۑ�����сA�Ăяo���@�\(�t�@�C�������w�肷��K�v����B
//     �܂��AVC�̃o�[�W�������Â��ꍇ��Windows Kit��DDK���C���X�g�[�����ĂȂ��Ɠ����Ȃ��ꍇ������܂��B
//     ���̏ꍇ��cserial.h����(36�s)
//     #define __NOT_USE_SEARCH_COMPORT
//     �̃R�����g�A�E�g���������Ă��������B�V���A���|�[�g�̔ԍ��𒲂ׂ�@�\���g���Ȃ��Ȃ�܂����R���p
//     �C���͂ł���Ǝv���܂��B(Ver.0.6�̋@�\�̖�����)�B
//      VC2012�AVC2013�ł͓���m�F�ς݁BVC6.0�ł�Ver0.6�̋@�\�͎g���܂���ł����B
//      (2014.10.18 Sat.)
// 0.71 ���ݐݒ肵�Ă���V���A���|�[�g�̖��O���擾�ł���悤�ɂ����B(2014.10.23 Thr.)
// 0.80 �X�g�b�v�r�b�g��p���e�B�̐ݒ���s����SetSerialPort2���\�b�h��ǉ������B(2017.4.13 Thr.)
// 0.81 Visual Studio 2017�ō�����v���W�F�N�g�ɑΉ��B(2017.12.15 Fri.)
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "cserial.h"

#ifndef __NOT_USE_SEARCH_COMPORT
#include <setupapi.h>
#include <cfgmgr32.h>
#pragma	comment(lib,"setupapi.lib")
#endif

//������
CSerial::CSerial( void ):
m_hwndParent( NULL ),
m_comHandle( NULL ),
m_sendbuffersize( 1024 ),
m_receivebuffersize( 1024 ),
m_baudrate( 2400 ),
m_senddata( NULL ),
m_receivedata( NULL ),
m_senddatasize( 1 ),
m_receivedatasize( 1 ),
m_condition( 0 ),
m_enable_com_num( 0 ),
m_timeout( 50 )
{
#ifdef __NOT_USE_SPRINTF_S
	_stprintf( m_com2 , TEXT( "COM1" ) );
	_stprintf( m_com , TEXT("\\\\.\\COM1") );
#else
	_stprintf_s( m_com2 , _countof(m_com2) , TEXT( "COM1" ) );
	_stprintf_s( m_com , _countof(m_com) , TEXT("\\\\.\\COM1") );
#endif
	return;
}

//�I������
CSerial::~CSerial( void )
{
	if( m_senddata )
	{
		delete [ ] m_senddata;
		m_senddata = NULL;
	}
	if( m_receivedata )
	{
		delete [ ] m_receivedata;
		m_receivedata = NULL;
	}
	if( m_condition )
	{
		CloseSerialPort( );
	}
}

//COM�|�[�g�̐ݒ�
//�߂�l ������ 1 �A���s�� 0
//��1���� �ݒ肷��COM�|�[�g�̕����� �� "COM1" �� "COM2" �Ȃ�
BOOL CSerial::SetSerialPortName( TCHAR *comname )
{
#ifdef __NOT_USE_SPRINTF_S
	if( _stprintf( m_com2 , TEXT( "%s" ) , comname ) )
	if( _stprintf( m_com , TEXT("\\\\.\\%s") , comname ) )
		return 1;
#else
	if( _stprintf_s( m_com2 , _countof(m_com2) , TEXT( "%s" ) , comname ) )
	if( _stprintf_s( m_com , _countof(m_com) , TEXT("\\\\.\\%s") , comname ) )
		return 1;
#endif
	return 0;
}

//�e�E�B���h�E�̐ݒ�
//�߂�l �Ȃ�
//��1���� �e�E�B���h�E�̃n���h��
void CSerial::SetHwnd( HWND hwnd )
{
	m_hwndParent = hwnd;
}

//�V���A���|�[�g�̃^�C���A�E�g�ݒ�
//�߂�l ������ 1 �A���s�� 0
//��1���� unsigned int�^ �^�C���A�E�g�܂ł̎���(msec)
BOOL CSerial::SetSerialTimeOut( unsigned int timeout )
{
	COMMTIMEOUTS timeouts;

	if( !GetCommTimeouts( m_comHandle , &timeouts ) )
	{
		#ifndef __NO_SRIAL_MESSAGE_BOX__
		MessageBox( m_hwndParent , TEXT("�V���A���|�[�g�̃^�C���A�E�g�̐ݒ���擾�ł��܂���ł����B") , TEXT("error") , MB_OK );
		#endif
		return 0;
	}

	// �V���A���|�[�g�̃f�[�^��1byte��M����Ƃ��҂�����(�P�ʂ�ms)
	timeouts.ReadIntervalTimeout		=timeout;
	// �V���A���|�[�g�̃f�[�^��2byte�ȏ��M����Ƃ�1����������̏旦
	timeouts.ReadTotalTimeoutMultiplier	=0;
	// �V���A���|�[�g�̃f�[�^����M����Ƃ��̃g�[�^���̑҂�����(�P�ʂ�ms)
	timeouts.ReadTotalTimeoutConstant	=timeout;
	// �V���A���|�[�g�̃f�[�^��2byte�ȏ㑗�M����Ƃ�1����������̏旦
	timeouts.WriteTotalTimeoutMultiplier=0;
	// �V���A���|�[�g�̃f�[�^�𑗐M����Ƃ��̃g�[�^���̑҂�����(�P�ʂ�ms)
	timeouts.WriteTotalTimeoutConstant	=timeout;

	if( !SetCommTimeouts( m_comHandle , &timeouts ) )
	{
		#ifndef __NO_SRIAL_MESSAGE_BOX__
		MessageBox( m_hwndParent , TEXT("�V���A���|�[�g�̃^�C���A�E�g�̐ݒ肪�ł��܂���ł����B") , TEXT("error") , MB_OK );
		#endif
		return 0;
	}

	m_timeout = timeout;

	return 1;
}

//�V���A���|�[�g�̐ݒ�
//�߂�l ������ 1 �A���s�� 0
// ��1���� unsigned int�^ �V���A���ʐM�̃{�[���C�g(bps)
// ��2���� unsigned int�^ ���M�pFIFO�������̃T�C�Y(byte)
// ��3���� unsigned int�^ ��M�pFIFO�������̃T�C�Y(byte)
BOOL CSerial::SetSerialPort( unsigned int baudrate , unsigned int send_buffer_size , unsigned int receive_buffer_size )
{
	DCB		dcb;

	// �V���A���|�[�g�̏�Ԃ��擾����B
	if( !GetCommState( m_comHandle , &dcb ) )
	{
		#ifndef __NO_SRIAL_MESSAGE_BOX__
		::MessageBox( m_hwndParent , TEXT("�V���A���|�[�g�̏�Ԃ��擾�ł��܂���ł����B") , TEXT("error") , MB_OK );
		#endif
		return 0;
	}

	// �{�[���C�g�̐ݒ�(�P�ʂ�bps{bit per second})
	dcb.BaudRate		=baudrate;
	// �f�[�^�̃r�b�g��(�P�ʂ�bit)
	dcb.ByteSize		=8;
	// �p���e�B�̐ݒ�
	dcb.Parity			=NOPARITY;
	// �X�g�b�v�r�b�g�̐�
	dcb.StopBits		=ONESTOPBIT;
	// ��̓t���[����֌W
	dcb.fOutX			=false;
	dcb.fInX			=false;
	dcb.fTXContinueOnXoff =false;

	// �V���A���|�[�g�̏�Ԃ�ݒ肷��B
	if( !SetCommState( m_comHandle , &dcb ) )
	{
		#ifndef __NO_SRIAL_MESSAGE_BOX__
		::MessageBox( m_hwndParent , TEXT("�V���A���|�[�g�̏�Ԃ�ݒ�ł��܂���ł����B") , TEXT("error") , MB_OK );
		#endif
		return 0;
	}

	m_baudrate = baudrate;

	// �V���A���|�[�g�̃o�b�t�@��ݒ肷��B
	if( !SetupComm( m_comHandle , receive_buffer_size , send_buffer_size ) )
	{
		#ifndef __NO_SRIAL_MESSAGE_BOX__
		::MessageBox( m_hwndParent , TEXT("�V���A���|�[�g�̃o�b�t�@�̐ݒ�����s���܂����B") , TEXT("error") , MB_OK );
		#endif
		return 0;
	}

	m_sendbuffersize = send_buffer_size;
	m_receivebuffersize = receive_buffer_size;

	// �t���[������s��Ȃ��悤�ɐݒ�
	EscapeCommFunction( m_comHandle , SETDTR | SETRTS );

	return 1;
}

//�V���A���|�[�g���I�[�v������B
//�߂�l ������ 1 �A���s�� 0
//���� �Ȃ�
BOOL CSerial::OpenSerialPort( void )
{

	// �V���A���|�[�g���J��
	m_comHandle = CreateFile(	m_com,
								GENERIC_READ | GENERIC_WRITE,
								0,
								0,
								OPEN_EXISTING,
								0,
								0);

	// �V���A���|�[�g���J���̂����s�����ꍇ
	if( m_comHandle == INVALID_HANDLE_VALUE )
	{
		m_comHandle = NULL;
		#ifndef __NO_SRIAL_MESSAGE_BOX__
		::MessageBox( m_hwndParent , TEXT("�V���A���|�[�g���J�����Ƃ��ł��܂���ł����B") , NULL , MB_OK );
		#endif
		return 0;
	}

	// �V���A���|�[�g�̃^�C���A�E�g�̐ݒ�
	if( !SetSerialTimeOut( m_timeout ) )
	{
		// �V���A���|�[�g�����B
		CloseSerialPort( );
		return 0;
	}

	// �V���A���|�[�g�̏�Ԃ̐ݒ�
	if( !SetSerialPort( m_baudrate , m_sendbuffersize , m_receivebuffersize ) )
	{
		// �V���A���|�[�g�����B
		CloseSerialPort( );
		return 0;
	}
	if( !m_senddata || !m_receivedata )
	{
		if( MakeBuffer( m_senddatasize , m_receivedatasize ) == 0 )
		{
			#ifndef __NO_SRIAL_MESSAGE_BOX__
			::MessageBox( m_hwndParent , TEXT("�V���A���ʐM�p����M�o�b�t�@���擾�ł��܂���ł����B") , NULL , MB_OK );
			#endif
			CloseSerialPort( );
			return 0;
		}
	}
	m_condition = 1;
	return 1;
}

//�V���A���|�[�g���N���[�Y����B
//�߂�l �Ȃ�
//���� �Ȃ�
void CSerial::CloseSerialPort( void )
{
	CloseHandle( m_comHandle );
	m_comHandle = NULL;
	m_condition = 0;
}

// �V���A���|�[�g�̃f�[�^�̑��M(��������)
// �߂�l �V���A���|�[�g���J���ĂȂ��Ƃ�-1 ,���M����f�[�^�̃o�b�t�@������Ȃ����-2,����ȊO�͎��ۂɑ��M�����o�C�g��
// ������ unsigned int�^ �������ރf�[�^�̃o�C�g��
int CSerial::SendSerialData( unsigned int data_size )
{
	// ���ۂɑ��M�����f�[�^���̐錾
	DWORD dumy;

	// �V���A���|�[�g���J���ĂȂ��ꍇ
	if( !m_comHandle )
	{
		#ifndef __NO_SRIAL_MESSAGE_BOX__
		::MessageBox( m_hwndParent , TEXT("�V���A���|�[�g���J���Ă܂���B") , NULL , MB_OK );
		#endif
		return -1;
	}

	// ���M�f�[�^�o�b�t�@�����M������byte����菭�Ȃ��ꍇ
	if( m_senddatasize < data_size )
	{
		#ifndef __NO_SRIAL_MESSAGE_BOX__
		::MessageBox( m_hwndParent , TEXT("���M�f�[�^�����������܂��B") , NULL , MB_OK );
		#endif
		return -2;
	}

	// �f�[�^�𑗐M����B
	WriteFile( m_comHandle , m_senddata , data_size , &dumy , NULL );

	// ���ۂɑ��M�����o�C�g����Ԃ��B
	return ( int ) dumy;
}

// �V���A���|�[�g�̃f�[�^�̎�M(�ǂݍ���)
// �߂�l �V���A���|�[�g���J���ĂȂ��Ƃ�-1 ,���M����f�[�^�̃o�b�t�@������Ȃ����-2,����ȊO�͎��ۂɓǂݍ��񂾃o�C�g��
// ������ unsigned int�^ �ǂݍ��ރf�[�^�̃o�C�g��
int CSerial::ReceiveSerialData( unsigned int data_size )
{
	// ���ۂɎ�M�����f�[�^���̐錾
	DWORD dumy;

	// �V���A���|�[�g���J���ĂȂ��ꍇ
	if( !m_comHandle )
	{
		#ifndef __NO_SRIAL_MESSAGE_BOX__
		::MessageBox( m_hwndParent , TEXT("�V���A���|�[�g���J���Ă܂���B") , NULL , MB_OK );
		#endif
		return -1;
	}

	// ��M�f�[�^�o�b�t�@����M������byte����菭�Ȃ��ꍇ
	if( m_receivedatasize < data_size )
	{
		#ifndef __NO_SRIAL_MESSAGE_BOX__
		::MessageBox( m_hwndParent , TEXT("��M�f�[�^�����������܂��B") , NULL , MB_OK );
		#endif
		return -2;
	}

	// �f�[�^����M����B
	ReadFile( m_comHandle , m_receivedata , data_size , &dumy , NULL );

	// ���ۂɎ�M�����o�C�g����Ԃ��B
	return ( int ) dumy;
}

// �V���A���|�[�g�̃o�b�t�@�̃N���A
// �߂�l �V���A���|�[�g���J���ĂȂ��Ƃ�-1 ���s�Ȃ�0 ����������0�ȊO
// ���� �Ȃ�
int CSerial::SerialPortBufferClear( void )
{
	// �V���A���|�[�g���J���ĂȂ��ꍇ
	if( !m_comHandle )
	{
		#ifndef __NO_SRIAL_MESSAGE_BOX__
		::MessageBox( m_hwndParent , TEXT("�V���A���|�[�g���J���Ă܂���B") , NULL , MB_OK );
		#endif
		return -1;
	}

	// �o�b�t�@���N���A����(���M�A��M�̗����̃o�b�t�@)�B
	return PurgeComm( m_comHandle , PURGE_RXCLEAR | PURGE_TXCLEAR );
}

// ����M���邽�߂̃o�b�t�@���쐬����B
// �߂�l ������1�A���s��0
// ��1���� ���M���邽�߂̃o�b�t�@�T�C�Y
// ��2���� ��M���邽�߂̃o�b�t�@�T�C�Y
BOOL CSerial::MakeBuffer( unsigned int send_size , unsigned int receive_size )
{
	if( m_senddata )
	{
		delete [ ] m_senddata;
		m_senddata = NULL;
	}
	if( m_receivedata )
	{
		delete [ ] m_receivedata;
		m_receivedata = NULL;
	}
	m_senddata = new unsigned char[ send_size ];
	if( !m_senddata )
	{
		return 0;
	}
	m_senddatasize = send_size;
	m_receivedata = new unsigned char[ receive_size ];
	if( !m_receivedata )
	{
		return 0;
	}
	m_receivedatasize = receive_size;
	return 1;
}

// ��������Ver.0.2�p
// ��M�pFIFO�������̗ʂ��擾�B
// �߂�l �����݂�FIFO��M�������̗�(byte���P��)
// ���� �Ȃ�
int CSerial::GetInputFIFO( void )
{
	DWORD data;
	COMSTAT		comstat;
	ClearCommError(m_comHandle , &data , &comstat);
	return ( int ) comstat.cbInQue;
}

// ���M�pFIFO�������̗ʂ��擾�B
// �߂�l �����݂�FIFO���M�������̗�(byte���P��)
// ���� �Ȃ�
int CSerial::GetOutputFIFO( void )
{
	DWORD data;
	COMSTAT		comstat;
	ClearCommError(m_comHandle , &data , &comstat);
	return ( int ) comstat.cbOutQue;
}

//���L��ver0.6�p �������AVisualStudio�̃o�[�W�������Â��ƃR���p�C���G���[���ł�̂ł��̏ꍇ��cserial.h��"#define __NOT_USE_SEARCH_COMPORT"��L���ɂ��Ă��������B
// Windows DDK��WindowsKit������ƒʂ�ꍇ������܂��B VC2012�ȍ~�ł͕W���C���X�g�[���ł̓���m�F�ς݁B
#ifndef __NOT_USE_SEARCH_COMPORT
// �V���A���|�[�g�̐��𒲂ׂ�֐�
// �߂�l ���������L���ȃV���A���|�[�g�̐�
// ���� ������(TCHAR*) �����悤�Ƃ���V���A���|�[�g�̖��O(NULL���w�肵���ꍇ�͂��ׂẴV���A���|�[�g��T��)
int CSerial::GetComNum( TCHAR * c_name )
{
	TCHAR fname[ 512 ];
	DWORD i,j,k;
	TCHAR *p;
	DWORD Length = 0;
	SP_DEVINFO_DATA DeviceInfoData = {sizeof(SP_DEVINFO_DATA)}; /// �P���f�o�C�X���
	HDEVINFO hDevInfo = 0; // �񋓃f�o�C�X���
	TCHAR moji[1024];
	hDevInfo = SetupDiGetClassDevs( NULL, NULL, 0 , DIGCF_PRESENT | DIGCF_ALLCLASSES  );
	// hDevInfo = SetupDiGetClassDevs( &GUID_DEVINTERFACE_COMPORT, NULL, hwndDlg, ( DIGCF_PRESENT | DIGCF_DEVICEINTERFACE ) ); // ���ꂾ��Arduino UNO�����o����Ȃ�
	m_enable_com_num = 0;
	// �񋓂̏I���܂Ń��[�v
	for (i=0; SetupDiEnumDeviceInfo( hDevInfo, i, &DeviceInfoData ); i++) 
	{

		SetupDiGetDeviceRegistryProperty( hDevInfo, &DeviceInfoData, SPDRP_CLASS, NULL, (BYTE*)fname,sizeof(TCHAR)*512,&Length );
		#ifdef __NOT_USE_SPRINTF_S
			_tcscpy( moji, fname);
		#else
			_tcscpy_s( moji, _countof(moji) , fname);
		#endif
		// COM�����o
		if ( _tcsstr(moji,TEXT("Ports")) > 0 ) 
		{

			// �f�o�C�X�̃X�e�[�^�X���擾
			ULONG status  = 0;
			ULONG problem = 0;
			CONFIGRET cr = CM_Get_DevNode_Status(&status, &problem, DeviceInfoData.DevInst, 0);

			if (cr == CR_SUCCESS) 
			{
				if ( problem != CM_PROB_DISABLED )
				{	// �f�o�C�X�������ɂȂ��Ă��Ȃ����`�F�b�N
					// �t�����h���l�[�����擾
					SetupDiGetDeviceRegistryProperty( hDevInfo, &DeviceInfoData, SPDRP_FRIENDLYNAME, NULL, (BYTE*)fname,sizeof(TCHAR)*512,&Length );
					if( c_name )
					{
						if( _tcsstr( fname , c_name ) )
						{
							if( p = _tcsstr( fname , TEXT("(COM") ) )
							{
								if( _tcsstr( p , TEXT( ")" ) ) )
								{
									j = m_enable_com_num * 20;
									#ifdef __NOT_USE_SPRINTF_S
										_stprintf( &m_com_port_name[ j ] , TEXT("COM") );
									#else
										_stprintf_s( &m_com_port_name[ j ] , 20 , TEXT("COM") );
									#endif
									j += 3;
									k = 0;
									while( p[ 4 + k ] != TEXT( ')' ) )
									{
										m_com_port_name[ j++ ] = p[ 4 + k++ ];
									}
									m_com_port_name[ j ] = TEXT('\0');
									//MessageBox( 0 , &m_com_port_name[(m_enable_com_num)* 20] , 0 , MB_OK);
									m_enable_com_num++;
								}
							}
						}
					}
					else
					{
						if( p = _tcsstr( fname , TEXT("(COM") ) )
						{
							if( _tcsstr( p , TEXT( ")" ) ) )
							{
								j = m_enable_com_num * 20;
								#ifdef __NOT_USE_SPRINTF_S
									_stprintf( &m_com_port_name[ j ] , TEXT("COM") );
								#else
									_stprintf_s( &m_com_port_name[ j ] , 20 , TEXT("COM") );
								#endif
								j += 3;
								k = 0;
								while( p[ 4 + k ] != TEXT( ')' ) )
								{
									m_com_port_name[ j++ ] = p[ 4 + k++ ];
								}
								m_com_port_name[ j ] = TEXT('\0');
							//	MessageBox( 0 , &m_com_port_name[(m_enable_com_num)* 20] , 0 , MB_OK);
								m_enable_com_num++;
							}
						}
					}
				}
			}
		}
	}
	return m_enable_com_num;
}

// �V���A���|�[�g�̔ԍ����擾����֐�
// �߂�l ���������L���ȃV���A���|�[�g�̐�
// ��1���� ������(TCHAR*) �����悤�Ƃ���V���A���|�[�g�̖��O(NULL���w�肵���ꍇ�͂��ׂẴV���A���|�[�g��T��)
// ��2���� int�^ ���ԖڂɌ�������̖��O���擾���邩�H�@0���ŏ��Ɍ���������
// ��3���� ������(TCHAR*) �擾�����V���A���|�[�g�̖��O(COMXX)���i�[����ꏊ
int CSerial::GetComName( TCHAR * c_name , int num , TCHAR* com_name )
{
	GetComNum( c_name );
	if( m_enable_com_num > num )
	{
	//	if( m_enable_com_num != 0 )
		#ifdef __NOT_USE_SPRINTF_S
			_stprintf( com_name , TEXT("%s") , &m_com_port_name[ num * 20 ] );
		#else
			_stprintf_s( com_name , 20 , TEXT("%s") , &m_com_port_name[ num * 20 ] );
		#endif
	}
	return m_enable_com_num;
}

// �������V���A���|�[�g��ݒ肷��B
// �߂�l �ݒ�ł�����1 �ݒ�ł��Ȃ����0
// ���� int�^ �V���A���|�[�g�����������Ԃ̖��O��COM�|�[�g�Ƃ��Đݒ肷��B
int CSerial::SetFindSerialPortName( int num )
{
	//COM�|�[�g���������Ă��Ȃ��ꍇ�͈�x��������B
	if( m_enable_com_num == 0 )
	{
		GetComNum( NULL );
		//COM�|�[�g��������Ȃ��ꍇ��0��Ԃ��B
		if( m_enable_com_num == 0 )
			return 0;
	}
	if( m_enable_com_num > num )
	{
		SetSerialPortName( &m_com_port_name[ num * 20 ] );
		return 1;
	}
	return 0;
}

// �V���A���|�[�g�̖��O��T���čŏ��Ɍ�������COM�|�[�g���Z�b�g����֐�
// �����FTCHAR*�^ COM�|�[�g�̖��O���w�肷��BNULL�������"USB Serial Port"�Ɏw�肳���B�Ȃ�ł���������V���A���|�[�g��T�������Ȃ�"COM"�����
// �߂�l�F�ݒ肵���ꍇ��1�A���Ȃ�������0
BOOL CSerial::SearchSetSerialPortName( TCHAR* c_name )
{
	TCHAR fname[ 512 ];
	DWORD i,j,k;
	TCHAR *p;
	DWORD Length = 0;
	SP_DEVINFO_DATA DeviceInfoData = {sizeof(SP_DEVINFO_DATA)}; /// �P���f�o�C�X���
	HDEVINFO hDevInfo = 0; // �񋓃f�o�C�X���
	TCHAR moji[1024];
	TCHAR moji2[ 500 ];
	TCHAR moji3[ 20 ];
	if( c_name == 0 )
	{
		#ifdef __NOT_USE_SPRINTF_S
			_stprintf( moji2 , TEXT("USB Serial Port") );
		#else
			_stprintf_s( moji2 , _countof( moji2 ) , TEXT("USB Serial Port") );
		#endif
	}
	else
	{
		#ifdef __NOT_USE_SPRINTF_S
			_stprintf( moji2 , TEXT("%s") , c_name );
		#else
			_stprintf_s( moji2 , _countof( moji2 ) , TEXT("%s") , c_name );
		#endif
	}
	hDevInfo = SetupDiGetClassDevs( NULL, NULL, 0 , DIGCF_PRESENT | DIGCF_ALLCLASSES  );
	// hDevInfo = SetupDiGetClassDevs( &GUID_DEVINTERFACE_COMPORT, NULL, hwndDlg, ( DIGCF_PRESENT | DIGCF_DEVICEINTERFACE ) ); // ���ꂾ��Arduino UNO�����o����Ȃ�
	// �񋓂̏I���܂Ń��[�v
	for (i=0; SetupDiEnumDeviceInfo( hDevInfo, i, &DeviceInfoData ); i++) 
	{

		SetupDiGetDeviceRegistryProperty( hDevInfo, &DeviceInfoData, SPDRP_CLASS, NULL, (BYTE*)fname,sizeof(TCHAR)*512,&Length );
		#ifdef __NOT_USE_SPRINTF_S
			_tcscpy( moji, fname);
		#else
			_tcscpy_s( moji, _countof(moji) , fname );
		#endif
		// COM�����o
		if ( _tcsstr(moji,TEXT("Ports")) > 0 ) 
		{

			// �f�o�C�X�̃X�e�[�^�X���擾
			ULONG status  = 0;
			ULONG problem = 0;
			CONFIGRET cr = CM_Get_DevNode_Status(&status, &problem, DeviceInfoData.DevInst, 0);

			if (cr == CR_SUCCESS) 
			{
				if ( problem != CM_PROB_DISABLED )
				{	// �f�o�C�X�������ɂȂ��Ă��Ȃ����`�F�b�N
					// �t�����h���l�[�����擾
					SetupDiGetDeviceRegistryProperty( hDevInfo, &DeviceInfoData, SPDRP_FRIENDLYNAME, NULL, (BYTE*)fname,sizeof(TCHAR)*512,&Length );
					if( _tcsstr( fname , moji2 ) )
					{
						if( p = _tcsstr( fname , TEXT("(COM") ) )
						{
							if( _tcsstr( p , TEXT( ")" ) ) )
							{
								#ifdef __NOT_USE_SPRINTF_S
									_stprintf( moji3 , TEXT("COM") );
								#else
									_stprintf_s( moji3 , _countof(moji3) , TEXT("COM") );
								#endif
								j = 3;
								k = 0;
								while( p[ 4 + k ] != TEXT( ')' ) )
								{
									moji3[ j++ ] = p[ 4 + k++ ];
								}
								moji3[ j ] = TEXT('\0');
								SetSerialPortName( moji3 );
								return 1;
								//MessageBox( 0 , &m_com_port_name[(m_enable_com_num)* 20] , 0 , MB_OK);
							}
						}

					}
				}
			}
		}
	}
	return 0;
}
#endif

//�ȉ�Ver.0.7�̊֐�
//�Ō�ɊJ����COM�ԍ����t�@�C���ŕۑ�����֐�(�ۑ������t�@�C���̓��������Ō���܂��B�������ŕύX����Ɠǂ߂Ȃ��Ȃ邩������Ȃ��̂Ŕԍ���ύX����ȊO�͒���)
//����:TCHAR*�^ �t�@�C����(�g���q�t)����͂���B
//�߂�l:�t�@�C����ۑ��ł�����1�A�ł��Ȃ����0
BOOL CSerial::SaveCOMFile( TCHAR* FileName )
{
	FILE *fp;
#ifdef _UNICODE
	#ifdef __NOT_USE_SPRINTF_S
		fp = _tfopen( FileName , TEXT("w,ccs=UNICODE") );
		if( fp == 0 )
			return 0;
	#else
		errno_t error;
		error = _tfopen_s( &fp , FileName , TEXT("w,ccs=UNICODE") );
		if( error != 0 )
			return 0;
	#endif
#else
	#ifdef __NOT_USE_SPRINTF_S
		fp = _tfopen( FileName , TEXT("w") );
		if( fp == 0 )
			return 0;
	#else
		errno_t error;
		error = _tfopen_s( &fp , FileName , TEXT("w") );
		if( error != 0 )
			return 0;
	#endif
#endif

#ifdef __NOT_USE_SPRINTF_S
	_ftprintf( fp , TEXT("COM_NO=%s\n") , m_com2 );
#else
	_ftprintf_s( fp , TEXT("COM_NO=%s\n") , m_com2  );/*, _countof(m_com2)*/
#endif
	fclose( fp );
	return 1;
}

//�ۑ�����COM�ԍ���������Ă���t�@�C����ǂݍ����COM�|�[�g�̖��O�ɐݒ肷��֐�(������s������AOpenSerialPort�ŊJ���܂�)
//����:TCHAR*�^ �t�@�C����(�g���q�t)����͂���B
//�߂�l:�t�@�C����ǂ߂���1�A�ł��Ȃ����0
BOOL CSerial::LoadCOMFile( TCHAR* FileName )
{
	FILE *fp;
#ifdef _UNICODE
	#ifdef __NOT_USE_SPRINTF_S
		fp = _tfopen( FileName , TEXT("r,ccs=UNICODE") );
		if( fp == 0 )
			return 0;
	#else
		errno_t error;
		error = _tfopen_s( &fp , FileName , TEXT("r,ccs=UNICODE") );
		if( error != 0 )
			return 0;
	#endif
#else
	#ifdef __NOT_USE_SPRINTF_S
		fp = _tfopen( FileName , TEXT("r") );
		if( fp == 0 )
			return 0;
	#else
		errno_t error;
		error = _tfopen_s( &fp , FileName , TEXT("r") );
		if( error != 0 )
			return 0;
	#endif
#endif

#ifdef __NOT_USE_SPRINTF_S
	_ftscanf( fp , TEXT("COM_NO=%s\n") , m_com2 );
	_stprintf( m_com , TEXT("\\\\.\\%s") , m_com2 );
#else
	_ftscanf_s( fp , TEXT("COM_NO=%s\n") , m_com2 , _countof(m_com2) );
	_stprintf_s( m_com , _countof(m_com) , TEXT("\\\\.\\%s") , m_com2 );
#endif
	fclose( fp );
	return 1;
}

//�ȉ�Ver0.71
//�Ō��SetSerialPortName�Őݒ肵���V���A���|�[�g�̖��O���擾���郁�\�b�h
//�߂�l �Ȃ�
//��1���� TCHAR*�^ //���݂̃V���A���|�[�g�̖��O����͂�����������
void CSerial::GetSerialPortName( TCHAR *ComName )
{
#ifdef __NOT_USE_SPRINTF_S
	_stprintf( ComName , TEXT("%s") , m_com2 );
#else
	_stprintf_s( ComName , 20 , TEXT("%s") , m_com2 );
#endif
}

//�ȉ�Ver0.80
// COM�|�[�g�̐ݒ� �f�[�^�T�C�Y�A�p���e�B�A�X�g�b�v�r�b�g�̐ݒ肪�\
// �߂�l ������ 1 �A���s�� 0
// ��1���� unsigned int�^ �V���A���ʐM�̃{�[���C�g(bps)
// ��2���� unsigned int�^ ���M�pFIFO�������̃T�C�Y(byte)
// ��3���� unsigned int�^ ��M�pFIFO�������̃T�C�Y(byte)
// ��4���� unsigned int�^ �f�[�^�̃r�b�g�����w��(PIC�ŒʐM���s���ꍇ��8������)
// ��5���� unsigned int�^ �X�g�b�v�r�b�g�̐��̐ݒ�(1�X�g�b�v�r�b�g�Ȃ�0������B1.5�X�g�b�v�r�b�g�Ȃ�1������B2�X�g�b�v�r�b�g�Ȃ�2������BPIC�ŒʐM���s���ꍇ��1�X�g�b�v�r�b�g�Ȃ̂�0������B))
// ��6���� unsigned int�^ �p���e�B�̐ݒ�(�p���e�B�Ȃ��Ȃ�0�A�����p���e�B�Ȃ�2�A��p���e�B�Ȃ�1�A�}�[�N�p���e�B�Ȃ�3�A�X�y�[�X�p���e�B�Ȃ�4�APIC�ŒʐM���s���ꍇ�̓p���e�B�Ȃ��������̂�0������B)
BOOL CSerial::SetSerialPort2(unsigned int baudrate, unsigned int send_buffer_size, unsigned int receive_buffer_size , unsigned int data_size , unsigned int stopbits , unsigned int parity)
{
	DCB		dcb;

	// �V���A���|�[�g�̏�Ԃ��擾����B
	if (!GetCommState(m_comHandle, &dcb))
	{
#ifndef __NO_SRIAL_MESSAGE_BOX__
		::MessageBox(m_hwndParent, TEXT("�V���A���|�[�g�̏�Ԃ��擾�ł��܂���ł����B"), TEXT("error"), MB_OK);
#endif
		return 0;
	}

	// �{�[���C�g�̐ݒ�(�P�ʂ�bps{bit per second})
	dcb.BaudRate = baudrate;
	// �f�[�^�̃r�b�g��(�P�ʂ�bit)
	dcb.ByteSize = data_size;
	// �p���e�B�̐ݒ�
	dcb.Parity = parity;
	// �X�g�b�v�r�b�g�̐�
	dcb.StopBits = stopbits;
	// ��̓t���[����֌W
	dcb.fOutX = false;
	dcb.fInX = false;
	dcb.fTXContinueOnXoff = false;

	// �V���A���|�[�g�̏�Ԃ�ݒ肷��B
	if (!SetCommState(m_comHandle, &dcb))
	{
#ifndef __NO_SRIAL_MESSAGE_BOX__
		::MessageBox(m_hwndParent, TEXT("�V���A���|�[�g�̏�Ԃ�ݒ�ł��܂���ł����B"), TEXT("error"), MB_OK);
#endif
		return 0;
	}

	m_baudrate = baudrate;

	// �V���A���|�[�g�̃o�b�t�@��ݒ肷��B
	if (!SetupComm(m_comHandle, receive_buffer_size, send_buffer_size))
	{
#ifndef __NO_SRIAL_MESSAGE_BOX__
		::MessageBox(m_hwndParent, TEXT("�V���A���|�[�g�̃o�b�t�@�̐ݒ�����s���܂����B"), TEXT("error"), MB_OK);
#endif
		return 0;
	}

	m_sendbuffersize = send_buffer_size;
	m_receivebuffersize = receive_buffer_size;

	// �t���[������s��Ȃ��悤�ɐݒ�
	EscapeCommFunction(m_comHandle, SETDTR | SETRTS);

	return 1;
}