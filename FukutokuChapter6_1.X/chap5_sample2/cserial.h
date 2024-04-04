////////////////////////////////////////////////////////////////////////////////////////////////
// �V���A���ʐM�x���N���X Ver 0.81 
// cserial.h ����
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
#ifndef __CSERIAL_H__
#define __CSERIAL_H__

//������L���ɂ���ƃ��b�Z�[�W�{�b�N�X���\������Ȃ��Ȃ�܂��B
#define __NO_SRIAL_MESSAGE_BOX__

//sprintf_s�n���g�p���Ȃ��ꍇ�͉��̃R�����g��L���ɂ���B
//#define __NOT_USE_SPRINTF_S

//Windows DDK���������Ă��Ȃ����ŃR���p�C�����ł��Ȃ��ꍇ�͉��̃R�����g��L���ɂ���B�������A�L����COM�|�[�g�������Ă���Ȃ��Ȃ�܂��B
//#define __NOT_USE_SEARCH_COMPORT

//windows.h��include����ĂȂ����windows.h��include����B
#ifndef _WINDOWS_
#include < windows.h >
#endif

//stdio.h��include����ĂȂ����stdio.h��include����B
#ifndef _INC_STDIO
#include < stdio.h >
#endif

#include < tchar.h >

class CSerial
{
	// �{�N���X�𑕔�����e�n���h�� �����l0
	HWND m_hwndParent;
	// �{�N���X�Ŏg�p���Ă���n���h�� �����l0
	HANDLE m_comHandle;
	// ���M�p�o�b�t�@�T�C�Y(byte�P��) �����l1024
	unsigned int m_sendbuffersize;
	// ��M�p�o�b�t�@�T�C�Y(byte�P��) �����l1024
	unsigned int m_receivebuffersize;
	// �ʐM�{�[���C�g�p�ϐ�(bps) �����l2400
	unsigned int m_baudrate;
	// com�|�[�g�I��p������ �����l"COM1"
	TCHAR m_com[ 20 ];
	TCHAR m_com2[ 20 ];
	// �V���A���ʐM�p�N���X�̏�Ԃ�\���ϐ� �����l0
	unsigned char m_condition;
	// �V���A������M�̃^�C���A�E�g�܂ł̎���(msec) �����l50
	unsigned int m_timeout;
	// �V���A���|�[�g�̕�����
	TCHAR m_com_port_name[ 1200 ];
	// �T�����L���ȃV���A���|�[�g�̐�
	int m_enable_com_num;

public:
	// �V���A�����M�p�f�[�^(�|�C���^) �����l0
	unsigned char * m_senddata;
	// �V���A����M�p�f�[�^(�|�C���^) �����l0
	unsigned char * m_receivedata;
	// �V���A�����M�p�f�[�^�̐� �����l1
	unsigned int m_senddatasize;
	// �V���A����M�p�f�[�^ �����l1
	unsigned int m_receivedatasize;

	// ������
	CSerial( void );
	// �I������
	virtual ~CSerial( void );

	// �V���A���|�[�g�̖��O�̐ݒ�
	// �߂�l ������ 1 �A���s�� 0
	// ��1���� �ݒ肷��COM�|�[�g�̕����� �� "COM1" �� "COM2" �Ȃ�
	BOOL SetSerialPortName( TCHAR * );
	// �e�E�B���h�E�̐ݒ�
	// �߂�l �Ȃ�
	// ��1���� �e�E�B���h�E�̃n���h��


	void SetHwnd( HWND );
	// COM�|�[�g�̃^�C���A�E�g�ݒ�
	// �߂�l ������ 1 �A���s�� 0
	// ��1���� unsigned int�^ �^�C���A�E�g�܂ł̎���(msec)
	BOOL SetSerialTimeOut( unsigned int );
	// COM�|�[�g�̐ݒ�
	// �߂�l ������ 1 �A���s�� 0
	// ��1���� unsigned int�^ �V���A���ʐM�̃{�[���C�g(bps)
	// ��2���� unsigned int�^ ���M�pFIFO�������̃T�C�Y(byte)
	// ��3���� unsigned int�^ ��M�pFIFO�������̃T�C�Y(byte)
	BOOL SetSerialPort( unsigned int , unsigned int , unsigned int );
	// �V���A���|�[�g���I�[�v������B
	// �߂�l ������ 1 �A���s�� 0
	// ��1���� �Ȃ�
	BOOL OpenSerialPort( void );
	// �V���A���|�[�g���N���[�Y����B
	// �߂�l �Ȃ�
	// ���� �Ȃ�
	void CloseSerialPort( void );
	// �V���A���|�[�g�̃f�[�^�̑��M(��������)
	// �߂�l �V���A���|�[�g���J���ĂȂ��Ƃ�-1 ,���M����f�[�^�̃o�b�t�@������Ȃ����-2,����ȊO�͎��ۂɑ��M�����o�C�g��
	// ������ unsigned int�^ �������ރf�[�^�̃o�C�g��
	int SendSerialData( unsigned int );
	// �V���A���|�[�g�̃f�[�^�̎�M(�ǂݍ���)
	// �߂�l �V���A���|�[�g���J���ĂȂ��Ƃ�-1 ,���M����f�[�^�̃o�b�t�@������Ȃ����-2,����ȊO�͎��ۂɓǂݍ��񂾃o�C�g��
	// ������ unsigned int�^ �ǂݍ��ރf�[�^�̃o�C�g��
	int ReceiveSerialData( unsigned int );
	// �V���A���|�[�g�̃o�b�t�@�̃N���A
	// �߂�l �V���A���|�[�g���J���ĂȂ��Ƃ�-1 ���s�Ȃ�0 ����������0�ȊO
	// ���� �Ȃ�
	int SerialPortBufferClear( void );
	// ����M���邽�߂̃o�b�t�@���쐬����B
	// �߂�l ������1�A���s��0
	// ��1���� ���M���邽�߂̃o�b�t�@�T�C�Y
	// ��2���� ��M���邽�߂̃o�b�t�@�T�C�Y
	BOOL MakeBuffer( unsigned int , unsigned int );
	// ��M�pFIFO�������̗ʂ��擾�B
	// �߂�l �����݂�FIFO��M�������̗�(byte���P��)
	// ���� �Ȃ�
	int GetInputFIFO( void );
	// ���M�pFIFO�������̗ʂ��擾�B
	// �߂�l �����݂�FIFO���M�������̗�(byte���P��)
	// ���� �Ȃ�
	int GetOutputFIFO( void );
#ifndef __NOT_USE_SEARCH_COMPORT
	// �V���A���|�[�g�̐��𒲂ׂ�֐�
	// �߂�l ���������L���ȃV���A���|�[�g�̐�
	// ���� ������(TCHAR*) �����悤�Ƃ���V���A���|�[�g�̖��O(NULL���w�肵���ꍇ�͂��ׂẴV���A���|�[�g��T��)
	int GetComNum( TCHAR * );
	// �V���A���|�[�g�̔ԍ����擾����֐�
	// �߂�l ���������L���ȃV���A���|�[�g�̐�
	// ��1���� ������(TCHAR*) �����悤�Ƃ���V���A���|�[�g�̖��O(NULL���w�肵���ꍇ�͂��ׂẴV���A���|�[�g��T��)
	// ��2���� int�^ ���ԖڂɌ�������̖��O���擾���邩�H
	// ��3���� ������(TCHAR*) �擾�����V���A���|�[�g�̖��O(COMXX)���i�[����ꏊ
	int GetComName( TCHAR * , int , TCHAR* );
	// �������V���A���|�[�g��ݒ肷��B
	// �߂�l �ݒ�ł�����1 �ݒ�ł��Ȃ����0
	// ���� int�^ �V���A���|�[�g�����������Ԃ̖��O��COM�|�[�g�Ƃ��Đݒ肷��B
	int SetFindSerialPortName( int );
	// �V���A���|�[�g�̖��O��T���čŏ��Ɍ�������COM�|�[�g���Z�b�g����֐�
	// �����FTCHAR*�^ COM�|�[�g�̖��O���w�肷��BNULL�������"USB Serial Port"�Ɏw�肳���B�Ȃ�ł���������V���A���|�[�g��T�������Ȃ�"COM"�����
	// �߂�l�F�ݒ肵���ꍇ��1�A���Ȃ�������0
	BOOL SearchSetSerialPortName( TCHAR* DevName );
#endif
	//�Ō�ɊJ����COM�ԍ����t�@�C���ŕۑ�����֐�(�ۑ������t�@�C���̓��������Ō���܂��B�������ŕύX����Ɠǂ߂Ȃ��Ȃ邩������Ȃ��̂Ŕԍ���ύX����ȊO�͒���)
	//����:TCHAR*�^ �t�@�C����(�g���q�t)����͂���B
	//�߂�l:�t�@�C����ۑ��ł�����1�A�ł��Ȃ����0
	BOOL SaveCOMFile( TCHAR* FileName );
	//�ۑ�����COM�ԍ���������Ă���t�@�C����ǂݍ����COM�|�[�g�̖��O�ɐݒ肷��֐�(������s������AOpenSerialPort�ŊJ���܂�)
	//����:TCHAR*�^ �t�@�C����(�g���q�t)����͂���B
	//�߂�l:�t�@�C����ǂ߂���1�A�ł��Ȃ����0
	BOOL LoadCOMFile( TCHAR* FileName );
	//�Ō��SetSerialPortName�Őݒ肵���V���A���|�[�g�̖��O���擾���郁�\�b�h
	//�߂�l �Ȃ�
	//��1���� TCHAR*�^ //���݂̃V���A���|�[�g�̖��O����͂�����������
	void GetSerialPortName( TCHAR *ComName ); 
	// COM�|�[�g�̐ݒ� �f�[�^�T�C�Y�A�p���e�B�A�X�g�b�v�r�b�g�̐ݒ肪�\
	// �߂�l ������ 1 �A���s�� 0
	// ��1���� unsigned int�^ �V���A���ʐM�̃{�[���C�g(bps)
	// ��2���� unsigned int�^ ���M�pFIFO�������̃T�C�Y(byte)
	// ��3���� unsigned int�^ ��M�pFIFO�������̃T�C�Y(byte)
	// ��4���� unsigned int�^ �f�[�^�̃r�b�g�����w��(�ʏ�8)
	// ��5���� unsigned int�^ �X�g�b�v�r�b�g�̐��̐ݒ�(1�X�g�b�v�r�b�g�Ȃ�0������B1.5�X�g�b�v�r�b�g�Ȃ�1������B2�X�g�b�v�r�b�g�Ȃ�2������B)
	// ��6���� unsigned int�^ �p���e�B�̐ݒ�(�p���e�B�Ȃ��Ȃ�0�A�����p���e�B�Ȃ�2�A��p���e�B�Ȃ�1�A�}�[�N�p���e�B�Ȃ�3�A�X�y�[�X�p���e�B�Ȃ�4)
	BOOL SetSerialPort2(unsigned int baudrate, unsigned int send_buffer_size, unsigned int receive_buffer_size, unsigned int data_size, unsigned int stopbits, unsigned int parity);

};

#endif