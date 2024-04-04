#include <stdio.h>
#pragma warning( disable : 4996 )
#include "cserial.h" //�V���A���ʐM�p�N���X�̃w�b�_�t�@�C��

void main(void)
{
	int data = 0;
	CSerial* cserial; //CSerial�N���X�̃|�C���^�̐錾
	cserial = new CSerial; //Cserial�N���X���擾
	cserial->MakeBuffer(1, 1); //���M�p�f�[�^��1byte�A��M�p�f�[�^��1byte�p�ӂ���B
	cserial->SetSerialPortName(TEXT("COM3")); //�p�\�R���̃V���A���|�[�g��ݒ肷��B�����̃p�\�R���̃f�o�C�X�}�l�[�W���Ŋm�F���邱�ƁB
	cserial->OpenSerialPort(); //�V���A���|�[�g���I�[�v������B
	cserial->SetSerialPort(2400, 1024, 1024); // �{�[���C�g�̐ݒ�B�����Ń{�[���C�g��2400�ɂ��Ă���B
	cserial->SerialPortBufferClear(); //�V���A���|�[�g�̑���MFIFO���������N���A����B
	//�}�C�i�X�̐��l����͂���ƃv���O�����I��
	while (data >= 0)
	{
		printf("dsPIC�ɑ��M�������f�[�^�����ĉ������B\n");
		scanf("%d", &data);
		cserial->m_senddata[0] = (unsigned char)data; //���M�p�f�[�^����
		cserial->SendSerialData(1); //�p�\�R������dsPIC��1byte�𑗐M
		cserial->ReceiveSerialData(1); //�p�\�R���ɗ��Ă���V���A���f�[�^��1byte��M
		printf("��M�����f�[�^�� %d �ł��B", cserial->m_receivedata[0]); //��M�����f�[�^��\��
	}
	cserial->CloseSerialPort(); //�V���A���|�[�g���N���[�Y����B
	delete cserial; //CSerial�N���X���J��
}
