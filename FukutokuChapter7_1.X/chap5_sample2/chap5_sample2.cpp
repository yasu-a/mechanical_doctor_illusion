#include <stdio.h>
#pragma warning( disable : 4996 )
#include "cserial.h" //�V���A���ʐM�p�N���X�̃w�b�_�t�@�C��
#include <cstdint>

char scan_int_as_char(void) {
    int value;
    scanf("%d", &value);
    return (char)value;
}

void main(void)
{
    int data = 0;
    CSerial *cserial; //CSerial�N���X�̃|�C���^�̐錾
    cserial = new CSerial; //Cserial�N���X���擾
    cserial->MakeBuffer(32, 32); //���M�p�f�[�^��1byte�A��M�p�f�[�^��1byte�p�ӂ���B
    cserial->SetSerialPortName(TEXT("COM5")); //�p�\�R���̃V���A���|�[�g��ݒ肷��B�����̃p�\�R���̃f�o�C�X�}�l�[�W���Ŋm�F���邱�ƁB
    cserial->OpenSerialPort(); //�V���A���|�[�g���I�[�v������B
    cserial->SetSerialPort(9600, 1024, 1024); // �{�[���C�g�̐ݒ�B�����Ń{�[���C�g��2400�ɂ��Ă���B
    cserial->SerialPortBufferClear(); //�V���A���|�[�g�̑���MFIFO���������N���A����B
    //�}�C�i�X�̐��l����͂���ƃv���O�����I��
    // while (data >= 0)
    // {
    //     printf("dsPIC�ɑ��M�������f�[�^�����ĉ������B\n");
    //     scanf("%d", &data);
    //     printf("SEND: %d\n", data);
    //     cserial->m_senddata[0] = (unsigned char)data; //���M�p�f�[�^����
    //     cserial->SendSerialData(1); //�p�\�R������dsPIC��1byte�𑗐M
    //     int recv_size = cserial->ReceiveSerialData(1); //�p�\�R���ɗ��Ă���V���A���f�[�^��1byte��M
    //     if (recv_size > 0) {
    //         printf("RECV %d\n", cserial->m_receivedata[0]); //��M�����f�[�^��\��
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

    cserial->CloseSerialPort(); //�V���A���|�[�g���N���[�Y����B
    delete cserial; //CSerial�N���X���J��
}