#include <stdio.h>
#pragma warning( disable : 4996 )
#include "cserial.h" //シリアル通信用クラスのヘッダファイル

void main(void)
{
	int data = 0;
	CSerial* cserial; //CSerialクラスのポインタの宣言
	cserial = new CSerial; //Cserialクラスを取得
	cserial->MakeBuffer(1, 1); //送信用データを1byte、受信用データを1byte用意する。
	cserial->SetSerialPortName(TEXT("COM3")); //パソコンのシリアルポートを設定する。自分のパソコンのデバイスマネージャで確認すること。
	cserial->OpenSerialPort(); //シリアルポートをオープンする。
	cserial->SetSerialPort(2400, 1024, 1024); // ボーレイトの設定。ここでボーレイトを2400にしている。
	cserial->SerialPortBufferClear(); //シリアルポートの送受信FIFOメモリをクリアする。
	//マイナスの数値を入力するとプログラム終了
	while (data >= 0)
	{
		printf("dsPICに送信したいデータを入れて下さい。\n");
		scanf("%d", &data);
		cserial->m_senddata[0] = (unsigned char)data; //送信用データを代入
		cserial->SendSerialData(1); //パソコンからdsPICに1byteを送信
		cserial->ReceiveSerialData(1); //パソコンに来ているシリアルデータを1byte受信
		printf("受信したデータは %d です。", cserial->m_receivedata[0]); //受信したデータを表示
	}
	cserial->CloseSerialPort(); //シリアルポートをクローズする。
	delete cserial; //CSerialクラスを開放
}
