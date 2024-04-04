#include <stdio.h>
#pragma warning( disable : 4996 )
#include "cserial.h" //シリアル通信用クラスのヘッダファイル
#include <cstdint>

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
        printf("Interval[ms]: ");
        int input;
        scanf("%d", &input);
        uint16_t interval = input;
        cserial->m_senddata[0] = 0x55;
        cserial->m_senddata[1] = 0xaa;
        cserial->m_senddata[2] = (interval >> 0) & 0x00ff;
        cserial->m_senddata[3] = (interval >> 8) & 0x00ff;
        cserial->SendSerialData(4);

        printf("...");
        for (;;) {
            putchar('.');
            Sleep(100);

            int n_recv = cserial->ReceiveSerialData(1);
            if (n_recv < 0) {
                return;
            }
            if (n_recv == 0) {
                continue;
            }
            if (n_recv != 1 || cserial->m_receivedata[0] != 0x55) {
                continue;
            }

            n_recv = cserial->ReceiveSerialData(1);
            if (n_recv < 0) {
                return;
            }
            if (n_recv == 0) {
                continue;
            }
            if (n_recv != 1 || cserial->m_receivedata[0] != 0xaa) {
                continue;
            }

            n_recv = cserial->ReceiveSerialData(4);
            if (n_recv != 4) {
                break;
            }
            uint32_t res = 0;
            for (int i = 0; i < 4; i++) {
                res <<= 8;
                res |= cserial->m_receivedata[i];
            }
            printf("\n%.3lf ms\n", (double)res / 1000.0);
            break;
        }
    }

    cserial->CloseSerialPort(); //シリアルポートをクローズする。
    delete cserial; //CSerialクラスを開放
}