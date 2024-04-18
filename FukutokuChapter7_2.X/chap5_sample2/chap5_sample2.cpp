#include <stdio.h>
#pragma warning( disable : 4996 )
#include "cserial.h" //�V���A���ʐM�p�N���X�̃w�b�_�t�@�C��
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

    cserial->CloseSerialPort(); //�V���A���|�[�g���N���[�Y����B
    delete cserial; //CSerial�N���X���J��
}