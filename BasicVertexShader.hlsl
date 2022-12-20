#include "BasicShaderHeader.hlsli"

Output BasicVS(
	float4 pos : POSITION,
	float4 normal : NORMAL,
	float2 uv : TEXCOORD,
	min16uint2 boneno : BONE_NO,
	min16uint weight : WEIGHT)
{
	Output output;//ピクセルシェーダーに渡す値
	pos= mul(world, pos);
	output.ray = pos.xyz - eye;

	//自分で加えたor変更した部分
	float4 posw = mul(world, pos); 
	output.svpos = mul(proj,mul(view, posw));
	//output.svpos = pos + 0.0001 * posw;
	//ここまで

	normal.w = 0; //平行移動成分を無効にする
	output.normal = mul(world, normal);
	output.vnormal = mul(view,output.normal); //法線にもワールド変換を行う
	output.uv = uv;
	return output;
}