////////////////////////////////////////////////////////////////////////////////////////////////
// シリアル通信支援クラス Ver 0.81 
// cserial.h 部分
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
#ifndef __CSERIAL_H__
#define __CSERIAL_H__

//ここを有効にするとメッセージボックスが表示されなくなります。
#define __NO_SRIAL_MESSAGE_BOX__

//sprintf_s系を使用しない場合は下のコメントを有効にする。
//#define __NOT_USE_SPRINTF_S

//Windows DDK等が入っていない等でコンパイルができない場合は下のコメントを有効にする。ただし、有効なCOMポートを見つけてくれなくなります。
//#define __NOT_USE_SEARCH_COMPORT

//windows.hがincludeされてなければwindows.hをincludeする。
#ifndef _WINDOWS_
#include < windows.h >
#endif

//stdio.hがincludeされてなければstdio.hをincludeする。
#ifndef _INC_STDIO
#include < stdio.h >
#endif

#include < tchar.h >

class CSerial
{
	// 本クラスを装備する親ハンドル 初期値0
	HWND m_hwndParent;
	// 本クラスで使用しているハンドル 初期値0
	HANDLE m_comHandle;
	// 送信用バッファサイズ(byte単位) 初期値1024
	unsigned int m_sendbuffersize;
	// 受信用バッファサイズ(byte単位) 初期値1024
	unsigned int m_receivebuffersize;
	// 通信ボーレイト用変数(bps) 初期値2400
	unsigned int m_baudrate;
	// comポート選択用文字列 初期値"COM1"
	TCHAR m_com[ 20 ];
	TCHAR m_com2[ 20 ];
	// シリアル通信用クラスの状態を表す変数 初期値0
	unsigned char m_condition;
	// シリアル送受信のタイムアウトまでの時間(msec) 初期値50
	unsigned int m_timeout;
	// シリアルポートの文字列
	TCHAR m_com_port_name[ 1200 ];
	// 探した有効なシリアルポートの数
	int m_enable_com_num;

public:
	// シリアル送信用データ(ポインタ) 初期値0
	unsigned char * m_senddata;
	// シリアル受信用データ(ポインタ) 初期値0
	unsigned char * m_receivedata;
	// シリアル送信用データの数 初期値1
	unsigned int m_senddatasize;
	// シリアル受信用データ 初期値1
	unsigned int m_receivedatasize;

	// 初期化
	CSerial( void );
	// 終了処理
	virtual ~CSerial( void );

	// シリアルポートの名前の設定
	// 戻り値 成功時 1 、失敗時 0
	// 第1引数 設定するCOMポートの文字列 例 "COM1" や "COM2" など
	BOOL SetSerialPortName( TCHAR * );
	// 親ウィンドウの設定
	// 戻り値 なし
	// 第1引数 親ウィンドウのハンドル


	void SetHwnd( HWND );
	// COMポートのタイムアウト設定
	// 戻り値 成功時 1 、失敗時 0
	// 第1引数 unsigned int型 タイムアウトまでの時間(msec)
	BOOL SetSerialTimeOut( unsigned int );
	// COMポートの設定
	// 戻り値 成功時 1 、失敗時 0
	// 第1引数 unsigned int型 シリアル通信のボーレイト(bps)
	// 第2引数 unsigned int型 送信用FIFOメモリのサイズ(byte)
	// 第3引数 unsigned int型 受信用FIFOメモリのサイズ(byte)
	BOOL SetSerialPort( unsigned int , unsigned int , unsigned int );
	// シリアルポートをオープンする。
	// 戻り値 成功時 1 、失敗時 0
	// 第1引数 なし
	BOOL OpenSerialPort( void );
	// シリアルポートをクローズする。
	// 戻り値 なし
	// 引数 なし
	void CloseSerialPort( void );
	// シリアルポートのデータの送信(書き込み)
	// 戻り値 シリアルポートが開いてないとき-1 ,送信するデータのバッファが足りなければ-2,それ以外は実際に送信したバイト数
	// 第一引数 unsigned int型 書き込むデータのバイト数
	int SendSerialData( unsigned int );
	// シリアルポートのデータの受信(読み込み)
	// 戻り値 シリアルポートが開いてないとき-1 ,送信するデータのバッファが足りなければ-2,それ以外は実際に読み込んだバイト数
	// 第一引数 unsigned int型 読み込むデータのバイト数
	int ReceiveSerialData( unsigned int );
	// シリアルポートのバッファのクリア
	// 戻り値 シリアルポートが開いてないとき-1 失敗なら0 成功したら0以外
	// 引数 なし
	int SerialPortBufferClear( void );
	// 送受信するためのバッファを作成する。
	// 戻り値 成功時1、失敗時0
	// 第1引数 送信するためのバッファサイズ
	// 第2引数 受信するためのバッファサイズ
	BOOL MakeBuffer( unsigned int , unsigned int );
	// 受信用FIFOメモリの量を取得。
	// 戻り値 今現在のFIFO受信メモリの量(byte数単位)
	// 引数 なし
	int GetInputFIFO( void );
	// 送信用FIFOメモリの量を取得。
	// 戻り値 今現在のFIFO送信メモリの量(byte数単位)
	// 引数 なし
	int GetOutputFIFO( void );
#ifndef __NOT_USE_SEARCH_COMPORT
	// シリアルポートの数を調べる関数
	// 戻り値 見つかった有効なシリアルポートの数
	// 引数 文字列(TCHAR*) 見つけようとするシリアルポートの名前(NULLを指定した場合はすべてのシリアルポートを探す)
	int GetComNum( TCHAR * );
	// シリアルポートの番号を取得する関数
	// 戻り値 見つかった有効なシリアルポートの数
	// 第1引数 文字列(TCHAR*) 見つけようとするシリアルポートの名前(NULLを指定した場合はすべてのシリアルポートを探す)
	// 第2引数 int型 何番目に見つけたやつの名前を取得するか？
	// 第3引数 文字列(TCHAR*) 取得したシリアルポートの名前(COMXX)を格納する場所
	int GetComName( TCHAR * , int , TCHAR* );
	// 見つけたシリアルポートを設定する。
	// 戻り値 設定できたら1 設定できなければ0
	// 引数 int型 シリアルポートを見つけた順番の名前をCOMポートとして設定する。
	int SetFindSerialPortName( int );
	// シリアルポートの名前を探して最初に見つかったCOMポートをセットする関数
	// 引数：TCHAR*型 COMポートの名前を指定する。NULLを入れると"USB Serial Port"に指定される。なんでもいいからシリアルポートを探したいなら"COM"を入力
	// 戻り値：設定した場合は1、しなかったら0
	BOOL SearchSetSerialPortName( TCHAR* DevName );
#endif
	//最後に開いたCOM番号をファイルで保存する関数(保存したファイルはメモ帳等で見れます。メモ帳で変更すると読めなくなるかもしれないので番号を変更する以外は注意)
	//引数:TCHAR*型 ファイル名(拡張子付)を入力する。
	//戻り値:ファイルを保存できたら1、できなければ0
	BOOL SaveCOMFile( TCHAR* FileName );
	//保存したCOM番号が書かれているファイルを読み込んでCOMポートの名前に設定する関数(これを行った後、OpenSerialPortで開きます)
	//引数:TCHAR*型 ファイル名(拡張子付)を入力する。
	//戻り値:ファイルを読めたら1、できなければ0
	BOOL LoadCOMFile( TCHAR* FileName );
	//最後にSetSerialPortNameで設定したシリアルポートの名前を取得するメソッド
	//戻り値 なし
	//第1引数 TCHAR*型 //現在のシリアルポートの名前を入力したい文字列
	void GetSerialPortName( TCHAR *ComName ); 
	// COMポートの設定 データサイズ、パリティ、ストップビットの設定が可能
	// 戻り値 成功時 1 、失敗時 0
	// 第1引数 unsigned int型 シリアル通信のボーレイト(bps)
	// 第2引数 unsigned int型 送信用FIFOメモリのサイズ(byte)
	// 第3引数 unsigned int型 受信用FIFOメモリのサイズ(byte)
	// 第4引数 unsigned int型 データのビット数を指定(通常8)
	// 第5引数 unsigned int型 ストップビットの数の設定(1ストップビットなら0を入れる。1.5ストップビットなら1を入れる。2ストップビットなら2を入れる。)
	// 第6引数 unsigned int型 パリティの設定(パリティなしなら0、偶数パリティなら2、奇数パリティなら1、マークパリティなら3、スペースパリティなら4)
	BOOL SetSerialPort2(unsigned int baudrate, unsigned int send_buffer_size, unsigned int receive_buffer_size, unsigned int data_size, unsigned int stopbits, unsigned int parity);

};

#endif