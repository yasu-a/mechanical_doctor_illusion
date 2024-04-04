#include <stdio.h>
#pragma warning( disable : 4996 )
#include "cserial.h" //シリアル通信用クラスのヘッダファイル

char scan_int_as_char(void) {
    int value;
    scanf("%d", &value);
    return (char)value;
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

    while (1) {
        int n_send = 0;
        printf("COMMAND ?> ");
        cserial->m_senddata[n_send++] = scan_int_as_char();
        if (cserial->m_senddata[0] <= 2) {  // command 0, 1, 2 requires parameter
            printf("ARG1 ?> ");
            cserial->m_senddata[n_send++] = scan_int_as_char();
        }
        cserial->SendSerialData(n_send);

        int n_recv = cserial->ReceiveSerialData(cserial->m_senddata[0] <= 2 ? 0 : 1);
        if (n_recv < 0) {
            break;
        }
        printf("RECV %d: ", n_recv);
        for (int i = 0; i < n_recv; i++) {
            printf("%02x ", (unsigned int)cserial->m_receivedata[i]);
        }
        putchar('\n');
    }

    cserial->CloseSerialPort(); //シリアルポートをクローズする。
    delete cserial; //CSerialクラスを開放
}