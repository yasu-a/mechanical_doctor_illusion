////////////////////////////////////////////////////////////////////////////////////////////////
// シリアル通信支援クラス Ver 0.81 
// cserial.cpp 部分
// 2017.12.15 Fri.
// 田中 基大作
// 0.1 初公開 (2004.5.28 Fri.)
// 0.11 ~CSerialメソッドに間違いがあったので修正 (2004.5.31 Mon.)
// 0.12 ReceiveSerialDataとMakeBufferメソッドに間違いがあったので修正
// 0.20 FIFOメモリの量を見ることが可能になる。(2006.6.9 Fri.)
// 0.3 COM10以降もサポート(2008.5.15 Thr.)
// 0.4 メッセージボックスを表示しない事も可能にした(ヘッダファイルで設定可能)。(2009.5.18 Mon.)
// 0.5 UNICODE 対応(2011.5.18 Wed.)
// 0.6 COMポート検索機能を追加、printf系の関数をpintf_sに変更(2014.9.15 Mon.)
// 0.7 sprintf系の関数をsprintf_sのみにしたのを選択できるように変更有効にするにはcserial.hの(33行)
//     #define __NOT_USE_SPRINTF_S
//     がコメントアウトしているのでこれのコメントアウトを解除すればよい。
//     最後に開いたシリアルポートのCOM番号を保存および、呼び出す機能(ファイル名を指定する必要あり。
//     また、VCのバージョンが古い場合はWindows KitかDDKをインストールしてないと動かない場合があります。
//     その場合はcserial.h内の(36行)
//     #define __NOT_USE_SEARCH_COMPORT
//     のコメントアウトを解除してください。シリアルポートの番号を調べる機能が使えなくなりますがコンパ
//     イルはできると思います。(Ver.0.6の機能の無効化)。
//      VC2012、VC2013では動作確認済み。VC6.0ではVer0.6の機能は使えませんでした。
//      (2014.10.18 Sat.)
// 0.71 現在設定しているシリアルポートの名前を取得できるようにした。(2014.10.23 Thr.)
// 0.80 ストップビットやパリティの設定を行えるSetSerialPort2メソッドを追加した。(2017.4.13 Thr.)
// 0.81 Visual Studio 2017で作ったプロジェクトに対応。(2017.12.15 Fri.)
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "cserial.h"

#ifndef __NOT_USE_SEARCH_COMPORT
#include <setupapi.h>
#include <cfgmgr32.h>
#pragma	comment(lib,"setupapi.lib")
#endif

//初期化
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

//終了処理
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

//COMポートの設定
//戻り値 成功時 1 、失敗時 0
//第1引数 設定するCOMポートの文字列 例 "COM1" や "COM2" など
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

//親ウィンドウの設定
//戻り値 なし
//第1引数 親ウィンドウのハンドル
void CSerial::SetHwnd( HWND hwnd )
{
	m_hwndParent = hwnd;
}

//シリアルポートのタイムアウト設定
//戻り値 成功時 1 、失敗時 0
//第1引数 unsigned int型 タイムアウトまでの時間(msec)
BOOL CSerial::SetSerialTimeOut( unsigned int timeout )
{
	COMMTIMEOUTS timeouts;

	if( !GetCommTimeouts( m_comHandle , &timeouts ) )
	{
		#ifndef __NO_SRIAL_MESSAGE_BOX__
		MessageBox( m_hwndParent , TEXT("シリアルポートのタイムアウトの設定を取得できませんでした。") , TEXT("error") , MB_OK );
		#endif
		return 0;
	}

	// シリアルポートのデータを1byte受信するとき待ち時間(単位はms)
	timeouts.ReadIntervalTimeout		=timeout;
	// シリアルポートのデータを2byte以上受信するとき1文字あたりの乗率
	timeouts.ReadTotalTimeoutMultiplier	=0;
	// シリアルポートのデータを受信するときのトータルの待ち時間(単位はms)
	timeouts.ReadTotalTimeoutConstant	=timeout;
	// シリアルポートのデータを2byte以上送信するとき1文字あたりの乗率
	timeouts.WriteTotalTimeoutMultiplier=0;
	// シリアルポートのデータを送信するときのトータルの待ち時間(単位はms)
	timeouts.WriteTotalTimeoutConstant	=timeout;

	if( !SetCommTimeouts( m_comHandle , &timeouts ) )
	{
		#ifndef __NO_SRIAL_MESSAGE_BOX__
		MessageBox( m_hwndParent , TEXT("シリアルポートのタイムアウトの設定ができませんでした。") , TEXT("error") , MB_OK );
		#endif
		return 0;
	}

	m_timeout = timeout;

	return 1;
}

//シリアルポートの設定
//戻り値 成功時 1 、失敗時 0
// 第1引数 unsigned int型 シリアル通信のボーレイト(bps)
// 第2引数 unsigned int型 送信用FIFOメモリのサイズ(byte)
// 第3引数 unsigned int型 受信用FIFOメモリのサイズ(byte)
BOOL CSerial::SetSerialPort( unsigned int baudrate , unsigned int send_buffer_size , unsigned int receive_buffer_size )
{
	DCB		dcb;

	// シリアルポートの状態を取得する。
	if( !GetCommState( m_comHandle , &dcb ) )
	{
		#ifndef __NO_SRIAL_MESSAGE_BOX__
		::MessageBox( m_hwndParent , TEXT("シリアルポートの状態を取得できませんでした。") , TEXT("error") , MB_OK );
		#endif
		return 0;
	}

	// ボーレイトの設定(単位はbps{bit per second})
	dcb.BaudRate		=baudrate;
	// データのビット数(単位はbit)
	dcb.ByteSize		=8;
	// パリティの設定
	dcb.Parity			=NOPARITY;
	// ストップビットの数
	dcb.StopBits		=ONESTOPBIT;
	// 後はフロー制御関係
	dcb.fOutX			=false;
	dcb.fInX			=false;
	dcb.fTXContinueOnXoff =false;

	// シリアルポートの状態を設定する。
	if( !SetCommState( m_comHandle , &dcb ) )
	{
		#ifndef __NO_SRIAL_MESSAGE_BOX__
		::MessageBox( m_hwndParent , TEXT("シリアルポートの状態を設定できませんでした。") , TEXT("error") , MB_OK );
		#endif
		return 0;
	}

	m_baudrate = baudrate;

	// シリアルポートのバッファを設定する。
	if( !SetupComm( m_comHandle , receive_buffer_size , send_buffer_size ) )
	{
		#ifndef __NO_SRIAL_MESSAGE_BOX__
		::MessageBox( m_hwndParent , TEXT("シリアルポートのバッファの設定を失敗しました。") , TEXT("error") , MB_OK );
		#endif
		return 0;
	}

	m_sendbuffersize = send_buffer_size;
	m_receivebuffersize = receive_buffer_size;

	// フロー制御を行わないように設定
	EscapeCommFunction( m_comHandle , SETDTR | SETRTS );

	return 1;
}

//シリアルポートをオープンする。
//戻り値 成功時 1 、失敗時 0
//引数 なし
BOOL CSerial::OpenSerialPort( void )
{

	// シリアルポートを開く
	m_comHandle = CreateFile(	m_com,
								GENERIC_READ | GENERIC_WRITE,
								0,
								0,
								OPEN_EXISTING,
								0,
								0);

	// シリアルポートを開くのを失敗した場合
	if( m_comHandle == INVALID_HANDLE_VALUE )
	{
		m_comHandle = NULL;
		#ifndef __NO_SRIAL_MESSAGE_BOX__
		::MessageBox( m_hwndParent , TEXT("シリアルポートを開くことができませんでした。") , NULL , MB_OK );
		#endif
		return 0;
	}

	// シリアルポートのタイムアウトの設定
	if( !SetSerialTimeOut( m_timeout ) )
	{
		// シリアルポートを閉じる。
		CloseSerialPort( );
		return 0;
	}

	// シリアルポートの状態の設定
	if( !SetSerialPort( m_baudrate , m_sendbuffersize , m_receivebuffersize ) )
	{
		// シリアルポートを閉じる。
		CloseSerialPort( );
		return 0;
	}
	if( !m_senddata || !m_receivedata )
	{
		if( MakeBuffer( m_senddatasize , m_receivedatasize ) == 0 )
		{
			#ifndef __NO_SRIAL_MESSAGE_BOX__
			::MessageBox( m_hwndParent , TEXT("シリアル通信用送受信バッファが取得できませんでした。") , NULL , MB_OK );
			#endif
			CloseSerialPort( );
			return 0;
		}
	}
	m_condition = 1;
	return 1;
}

//シリアルポートをクローズする。
//戻り値 なし
//引数 なし
void CSerial::CloseSerialPort( void )
{
	CloseHandle( m_comHandle );
	m_comHandle = NULL;
	m_condition = 0;
}

// シリアルポートのデータの送信(書き込み)
// 戻り値 シリアルポートが開いてないとき-1 ,送信するデータのバッファが足りなければ-2,それ以外は実際に送信したバイト数
// 第一引数 unsigned int型 書き込むデータのバイト数
int CSerial::SendSerialData( unsigned int data_size )
{
	// 実際に送信したデータ数の宣言
	DWORD dumy;

	// シリアルポートが開いてない場合
	if( !m_comHandle )
	{
		#ifndef __NO_SRIAL_MESSAGE_BOX__
		::MessageBox( m_hwndParent , TEXT("シリアルポートを開いてません。") , NULL , MB_OK );
		#endif
		return -1;
	}

	// 送信データバッファが送信したいbyte数より少ない場合
	if( m_senddatasize < data_size )
	{
		#ifndef __NO_SRIAL_MESSAGE_BOX__
		::MessageBox( m_hwndParent , TEXT("送信データ数が多すぎます。") , NULL , MB_OK );
		#endif
		return -2;
	}

	// データを送信する。
	WriteFile( m_comHandle , m_senddata , data_size , &dumy , NULL );

	// 実際に送信したバイト数を返す。
	return ( int ) dumy;
}

// シリアルポートのデータの受信(読み込み)
// 戻り値 シリアルポートが開いてないとき-1 ,送信するデータのバッファが足りなければ-2,それ以外は実際に読み込んだバイト数
// 第一引数 unsigned int型 読み込むデータのバイト数
int CSerial::ReceiveSerialData( unsigned int data_size )
{
	// 実際に受信したデータ数の宣言
	DWORD dumy;

	// シリアルポートが開いてない場合
	if( !m_comHandle )
	{
		#ifndef __NO_SRIAL_MESSAGE_BOX__
		::MessageBox( m_hwndParent , TEXT("シリアルポートを開いてません。") , NULL , MB_OK );
		#endif
		return -1;
	}

	// 受信データバッファが受信したいbyte数より少ない場合
	if( m_receivedatasize < data_size )
	{
		#ifndef __NO_SRIAL_MESSAGE_BOX__
		::MessageBox( m_hwndParent , TEXT("受信データ数が多すぎます。") , NULL , MB_OK );
		#endif
		return -2;
	}

	// データを受信する。
	ReadFile( m_comHandle , m_receivedata , data_size , &dumy , NULL );

	// 実際に受信したバイト数を返す。
	return ( int ) dumy;
}

// シリアルポートのバッファのクリア
// 戻り値 シリアルポートが開いてないとき-1 失敗なら0 成功したら0以外
// 引数 なし
int CSerial::SerialPortBufferClear( void )
{
	// シリアルポートが開いてない場合
	if( !m_comHandle )
	{
		#ifndef __NO_SRIAL_MESSAGE_BOX__
		::MessageBox( m_hwndParent , TEXT("シリアルポートを開いてません。") , NULL , MB_OK );
		#endif
		return -1;
	}

	// バッファをクリアする(送信、受信の両方のバッファ)。
	return PurgeComm( m_comHandle , PURGE_RXCLEAR | PURGE_TXCLEAR );
}

// 送受信するためのバッファを作成する。
// 戻り値 成功時1、失敗時0
// 第1引数 送信するためのバッファサイズ
// 第2引数 受信するためのバッファサイズ
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

// ここからVer.0.2用
// 受信用FIFOメモリの量を取得。
// 戻り値 今現在のFIFO受信メモリの量(byte数単位)
// 引数 なし
int CSerial::GetInputFIFO( void )
{
	DWORD data;
	COMSTAT		comstat;
	ClearCommError(m_comHandle , &data , &comstat);
	return ( int ) comstat.cbInQue;
}

// 送信用FIFOメモリの量を取得。
// 戻り値 今現在のFIFO送信メモリの量(byte数単位)
// 引数 なし
int CSerial::GetOutputFIFO( void )
{
	DWORD data;
	COMSTAT		comstat;
	ClearCommError(m_comHandle , &data , &comstat);
	return ( int ) comstat.cbOutQue;
}

//下記はver0.6用 ただし、VisualStudioのバージョンが古いとコンパイルエラーがでるのでその場合はcserial.hの"#define __NOT_USE_SEARCH_COMPORT"を有効にしてください。
// Windows DDKやWindowsKitを入れると通る場合もあります。 VC2012以降では標準インストールでの動作確認済み。
#ifndef __NOT_USE_SEARCH_COMPORT
// シリアルポートの数を調べる関数
// 戻り値 見つかった有効なシリアルポートの数
// 引数 文字列(TCHAR*) 見つけようとするシリアルポートの名前(NULLを指定した場合はすべてのシリアルポートを探す)
int CSerial::GetComNum( TCHAR * c_name )
{
	TCHAR fname[ 512 ];
	DWORD i,j,k;
	TCHAR *p;
	DWORD Length = 0;
	SP_DEVINFO_DATA DeviceInfoData = {sizeof(SP_DEVINFO_DATA)}; /// １件デバイス情報
	HDEVINFO hDevInfo = 0; // 列挙デバイス情報
	TCHAR moji[1024];
	hDevInfo = SetupDiGetClassDevs( NULL, NULL, 0 , DIGCF_PRESENT | DIGCF_ALLCLASSES  );
	// hDevInfo = SetupDiGetClassDevs( &GUID_DEVINTERFACE_COMPORT, NULL, hwndDlg, ( DIGCF_PRESENT | DIGCF_DEVICEINTERFACE ) ); // これだとArduino UNOが検出されない
	m_enable_com_num = 0;
	// 列挙の終わりまでループ
	for (i=0; SetupDiEnumDeviceInfo( hDevInfo, i, &DeviceInfoData ); i++) 
	{

		SetupDiGetDeviceRegistryProperty( hDevInfo, &DeviceInfoData, SPDRP_CLASS, NULL, (BYTE*)fname,sizeof(TCHAR)*512,&Length );
		#ifdef __NOT_USE_SPRINTF_S
			_tcscpy( moji, fname);
		#else
			_tcscpy_s( moji, _countof(moji) , fname);
		#endif
		// COMを検出
		if ( _tcsstr(moji,TEXT("Ports")) > 0 ) 
		{

			// デバイスのステータスを取得
			ULONG status  = 0;
			ULONG problem = 0;
			CONFIGRET cr = CM_Get_DevNode_Status(&status, &problem, DeviceInfoData.DevInst, 0);

			if (cr == CR_SUCCESS) 
			{
				if ( problem != CM_PROB_DISABLED )
				{	// デバイスが無効になっていないかチェック
					// フレンドリネームを取得
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

// シリアルポートの番号を取得する関数
// 戻り値 見つかった有効なシリアルポートの数
// 第1引数 文字列(TCHAR*) 見つけようとするシリアルポートの名前(NULLを指定した場合はすべてのシリアルポートを探す)
// 第2引数 int型 何番目に見つけたやつの名前を取得するか？　0が最初に見つけたもの
// 第3引数 文字列(TCHAR*) 取得したシリアルポートの名前(COMXX)を格納する場所
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

// 見つけたシリアルポートを設定する。
// 戻り値 設定できたら1 設定できなければ0
// 引数 int型 シリアルポートを見つけた順番の名前をCOMポートとして設定する。
int CSerial::SetFindSerialPortName( int num )
{
	//COMポートを検索していない場合は一度検索する。
	if( m_enable_com_num == 0 )
	{
		GetComNum( NULL );
		//COMポートが見つからない場合は0を返す。
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

// シリアルポートの名前を探して最初に見つかったCOMポートをセットする関数
// 引数：TCHAR*型 COMポートの名前を指定する。NULLを入れると"USB Serial Port"に指定される。なんでもいいからシリアルポートを探したいなら"COM"を入力
// 戻り値：設定した場合は1、しなかったら0
BOOL CSerial::SearchSetSerialPortName( TCHAR* c_name )
{
	TCHAR fname[ 512 ];
	DWORD i,j,k;
	TCHAR *p;
	DWORD Length = 0;
	SP_DEVINFO_DATA DeviceInfoData = {sizeof(SP_DEVINFO_DATA)}; /// １件デバイス情報
	HDEVINFO hDevInfo = 0; // 列挙デバイス情報
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
	// hDevInfo = SetupDiGetClassDevs( &GUID_DEVINTERFACE_COMPORT, NULL, hwndDlg, ( DIGCF_PRESENT | DIGCF_DEVICEINTERFACE ) ); // これだとArduino UNOが検出されない
	// 列挙の終わりまでループ
	for (i=0; SetupDiEnumDeviceInfo( hDevInfo, i, &DeviceInfoData ); i++) 
	{

		SetupDiGetDeviceRegistryProperty( hDevInfo, &DeviceInfoData, SPDRP_CLASS, NULL, (BYTE*)fname,sizeof(TCHAR)*512,&Length );
		#ifdef __NOT_USE_SPRINTF_S
			_tcscpy( moji, fname);
		#else
			_tcscpy_s( moji, _countof(moji) , fname );
		#endif
		// COMを検出
		if ( _tcsstr(moji,TEXT("Ports")) > 0 ) 
		{

			// デバイスのステータスを取得
			ULONG status  = 0;
			ULONG problem = 0;
			CONFIGRET cr = CM_Get_DevNode_Status(&status, &problem, DeviceInfoData.DevInst, 0);

			if (cr == CR_SUCCESS) 
			{
				if ( problem != CM_PROB_DISABLED )
				{	// デバイスが無効になっていないかチェック
					// フレンドリネームを取得
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

//以下Ver.0.7の関数
//最後に開いたCOM番号をファイルで保存する関数(保存したファイルはメモ帳等で見れます。メモ帳で変更すると読めなくなるかもしれないので番号を変更する以外は注意)
//引数:TCHAR*型 ファイル名(拡張子付)を入力する。
//戻り値:ファイルを保存できたら1、できなければ0
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

//保存したCOM番号が書かれているファイルを読み込んでCOMポートの名前に設定する関数(これを行った後、OpenSerialPortで開きます)
//引数:TCHAR*型 ファイル名(拡張子付)を入力する。
//戻り値:ファイルを読めたら1、できなければ0
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

//以下Ver0.71
//最後にSetSerialPortNameで設定したシリアルポートの名前を取得するメソッド
//戻り値 なし
//第1引数 TCHAR*型 //現在のシリアルポートの名前を入力したい文字列
void CSerial::GetSerialPortName( TCHAR *ComName )
{
#ifdef __NOT_USE_SPRINTF_S
	_stprintf( ComName , TEXT("%s") , m_com2 );
#else
	_stprintf_s( ComName , 20 , TEXT("%s") , m_com2 );
#endif
}

//以下Ver0.80
// COMポートの設定 データサイズ、パリティ、ストップビットの設定が可能
// 戻り値 成功時 1 、失敗時 0
// 第1引数 unsigned int型 シリアル通信のボーレイト(bps)
// 第2引数 unsigned int型 送信用FIFOメモリのサイズ(byte)
// 第3引数 unsigned int型 受信用FIFOメモリのサイズ(byte)
// 第4引数 unsigned int型 データのビット数を指定(PICで通信を行う場合は8が多い)
// 第5引数 unsigned int型 ストップビットの数の設定(1ストップビットなら0を入れる。1.5ストップビットなら1を入れる。2ストップビットなら2を入れる。PICで通信を行う場合は1ストップビットなので0を入れる。))
// 第6引数 unsigned int型 パリティの設定(パリティなしなら0、偶数パリティなら2、奇数パリティなら1、マークパリティなら3、スペースパリティなら4、PICで通信を行う場合はパリティなしが多いので0を入れる。)
BOOL CSerial::SetSerialPort2(unsigned int baudrate, unsigned int send_buffer_size, unsigned int receive_buffer_size , unsigned int data_size , unsigned int stopbits , unsigned int parity)
{
	DCB		dcb;

	// シリアルポートの状態を取得する。
	if (!GetCommState(m_comHandle, &dcb))
	{
#ifndef __NO_SRIAL_MESSAGE_BOX__
		::MessageBox(m_hwndParent, TEXT("シリアルポートの状態を取得できませんでした。"), TEXT("error"), MB_OK);
#endif
		return 0;
	}

	// ボーレイトの設定(単位はbps{bit per second})
	dcb.BaudRate = baudrate;
	// データのビット数(単位はbit)
	dcb.ByteSize = data_size;
	// パリティの設定
	dcb.Parity = parity;
	// ストップビットの数
	dcb.StopBits = stopbits;
	// 後はフロー制御関係
	dcb.fOutX = false;
	dcb.fInX = false;
	dcb.fTXContinueOnXoff = false;

	// シリアルポートの状態を設定する。
	if (!SetCommState(m_comHandle, &dcb))
	{
#ifndef __NO_SRIAL_MESSAGE_BOX__
		::MessageBox(m_hwndParent, TEXT("シリアルポートの状態を設定できませんでした。"), TEXT("error"), MB_OK);
#endif
		return 0;
	}

	m_baudrate = baudrate;

	// シリアルポートのバッファを設定する。
	if (!SetupComm(m_comHandle, receive_buffer_size, send_buffer_size))
	{
#ifndef __NO_SRIAL_MESSAGE_BOX__
		::MessageBox(m_hwndParent, TEXT("シリアルポートのバッファの設定を失敗しました。"), TEXT("error"), MB_OK);
#endif
		return 0;
	}

	m_sendbuffersize = send_buffer_size;
	m_receivebuffersize = receive_buffer_size;

	// フロー制御を行わないように設定
	EscapeCommFunction(m_comHandle, SETDTR | SETRTS);

	return 1;
}