#include <stdio.h>
#pragma warning( disable : 4996 )
#include "cserial.h" //シリアル通信用クラスのヘッダファイル
#include <cstdint>

char scan_int_as_char(void) {
    int value;
    scanf("%d", &value);
    return (char)value;
}

uint16_t read_u16(CSerial *s, int i) {
    return s->m_receivedata[i * 2] << 8 | s->m_receivedata[i * 2 + 1];
}

void main(void)
{
    int data = 0;
    CSerial *cserial; //CSerialクラスのポインタの宣言
    cserial = new CSerial; //Cserialクラスを取得
    cserial->MakeBuffer(32, 32); //送信用データを1byte、受信用データを1byte用意する。
    cserial->SetSerialPortName(TEXT("COM5")); //パソコンのシリアルポートを設定する。自分のパソコンのデバイスマネージャで確認すること。
    cserial->OpenSerialPort(); //シリアルポートをオープンする。
    cserial->SetSerialPort(9600, 1024, 1024); // ボーレイトの設定。ここでボーレイトを2400にしている。
    cserial->SerialPortBufferClear(); //シリアルポートの送受信FIFOメモリをクリアする。
    //マイナスの数値を入力するとプログラム終了
    // while (data >= 0)
    // {
    //     printf("dsPICに送信したいデータを入れて下さい。\n");
    //     scanf("%d", &data);
    //     printf("SEND: %d\n", data);
    //     cserial->m_senddata[0] = (unsigned char)data; //送信用データを代入
    //     cserial->SendSerialData(1); //パソコンからdsPICに1byteを送信
    //     int recv_size = cserial->ReceiveSerialData(1); //パソコンに来ているシリアルデータを1byte受信
    //     if (recv_size > 0) {
    //         printf("RECV %d\n", cserial->m_receivedata[0]); //受信したデータを表示
    //     }
    // }

    int n_send, n_recv;
    while (1) {
        Sleep(100);

        cserial->m_senddata[0] = 0;
        n_send = cserial->SendSerialData(1);
        if (n_send < 0) {
            break;
        }

        n_recv = cserial->ReceiveSerialData(2);
        if (read_u16(cserial, 0) != 0x55aa) {
            continue;
        }
        cserial->ReceiveSerialData(4);
        int a = (int16_t)read_u16(cserial, 0);
        int b = (int16_t)read_u16(cserial, 1);
        printf("AN0(%4d) + AN1(%4d) = %4d\r", a, b, a + b);
    }

    cserial->CloseSerialPort(); //シリアルポートをクローズする。
    delete cserial; //CSerialクラスを開放
}