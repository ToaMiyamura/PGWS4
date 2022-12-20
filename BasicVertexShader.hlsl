#include "BasicShaderHeader.hlsli"

Output BasicVS(
	float4 pos : POSITION,
	float4 normal : NORMAL,
	float2 uv : TEXCOORD,
	min16uint2 boneno : BONE_NO,
	min16uint weight : WEIGHT)
{
	Output output;//�s�N�Z���V�F�[�_�[�ɓn���l
	pos= mul(world, pos);
	output.ray = pos.xyz - eye;

	//�����ŉ�����or�ύX��������
	float4 posw = mul(world, pos); 
	output.svpos = mul(proj,mul(view, posw));
	//output.svpos = pos + 0.0001 * posw;
	//�����܂�

	normal.w = 0; //���s�ړ������𖳌��ɂ���
	output.normal = mul(world, normal);
	output.vnormal = mul(view,output.normal); //�@���ɂ����[���h�ϊ����s��
	output.uv = uv;
	return output;
}