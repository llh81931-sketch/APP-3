#ifndef NATIVESURFACE_MEMREAD_H
#define NATIVESURFACE_MEMREAD_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <malloc.h>
#include <math.h>
#include <thread>
#include <iostream>
#include <sys/stat.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <locale>
#include <string>
#include <codecvt>
#include <dlfcn.h>
#include <stdint.h>

// 仅内核驱动读写：RT + TWT（无 syscall / process_vm 回退）
#include "drivers/mem_backend.h"

#define PI 3.141592653589793238
typedef unsigned int ADDRESS;
typedef char PACKAGENAME;
typedef unsigned short UTF16;
typedef char UTF8;
typedef unsigned int UTF32;
#define UNI_SUR_HIGH_START (UTF32)0xD800
#define UNI_SUR_HIGH_END (UTF32)0xDBFF
#define UNI_SUR_LOW_START (UTF32)0xDC00
#define UNI_SUR_LOW_END (UTF32)0xDFFF
#define UNI_REPLACEMENT_CHAR (UTF32)0x0000FFFD
#define UNI_MAX_BMP (UTF32)0x0000FFFF
#define UNI_MAX_UTF16 (UTF32)0x0010FFFF
#define UNI_MAX_UTF32 (UTF32)0x7FFFFFFF
#define UNI_MAX_LEGAL_UTF32 (UTF32)0x0010FFFF

inline int pid = -1;
inline float matrix[16] = { 0 };
inline float angle = 0, camera = 0, r_x = 0, r_y = 0, r_w = 0;
inline float px = 0;
inline float py = 0;

// 初始化驱动：优先 TWT，其次 RT。失败返回 false（不做任何用户态内存回退）
inline bool MemDriver_Init() {
    if (!Mem().connect_auto()) return false;
    if (pid > 0) Mem().set_pid(pid);
    return Mem().ready();
}

// 1=TWT  2=RT
inline bool MemDriver_InitChoice(int choice) {
    bool ok = false;
    if (choice == 1) ok = Mem().connect_twt();
    else if (choice == 2) ok = Mem().connect_rt();
    else ok = Mem().connect_auto();
    if (!ok) return false;
    if (pid > 0) Mem().set_pid(pid);
    return Mem().ready();
}

inline const char* MemDriver_Name() {
    return Mem().name();
}

inline const char* MemDriver_Path() {
    return Mem().path();
}

inline bool MemDriver_Ready() {
    return Mem().ready();
}

// 进程 ID（包名）
inline int getProcessID(const char *packageName)
{
int id = MemBackend::find_pid_by_name(packageName);
// TWT 也可走驱动查 pid
if (id <= 0 && Mem().kind == MemBackendKind::Twt) {
int t = (int)Mem().twt.name_to_pid(packageName);
if (t > 0) id = t;
}
return id;
}

// 绑定目标进程到当前驱动
inline bool MemDriver_Attach(int target_pid) {
if (target_pid <= 0) return false;
if (!Mem().ready() && !Mem().connect_auto()) return false;
pid = target_pid;
Mem().set_pid(target_pid);
return true;
}

inline bool MemDriver_AttachName(const char* packageName) {
int id = getProcessID(packageName);
if (id <= 0) return false;
return MemDriver_Attach(id);
}

// 内核驱动读写（唯一路径）
inline bool vm_readv(unsigned long address, void *buffer, size_t size)
{
if (!Mem().ready() || pid <= 0 || !buffer || size == 0) return false;
return Mem().read((uintptr_t)address, buffer, size);
}

inline bool vm_writev(unsigned long address, void *buffer, size_t size)
{
if (!Mem().ready() || pid <= 0 || !buffer || size == 0) return false;
return Mem().write((uintptr_t)address, buffer, size);
}

// 兼容旧名 pvm：仅驱动，无 process_vm
inline bool pvm(void *address, void *buffer, size_t size, bool iswrite)
{
if (iswrite) return vm_writev((unsigned long)(uintptr_t)address, buffer, size);
return vm_readv((unsigned long)(uintptr_t)address, buffer, size);
}

inline float getfloat(unsigned long addr)
{
float var = 0;
vm_readv(addr, &var, 4);
return var;
}

inline float getFloat(unsigned long addr)
{
return getfloat(addr);
}

inline int getdword(unsigned long addr)
{
int var = 0;
vm_readv(addr, &var, 4);
return var;
}

inline int getDword(unsigned long addr)
{
return getdword(addr);
}

inline unsigned int getPtr32(unsigned int addr)
{
if (addr == 0) return 0;
unsigned int var = 0;
vm_readv(addr, &var, 4);
return var;
}

inline unsigned long getPtr64(unsigned long addr)
{
if (addr == 0) return 0;
unsigned long var = 0;
vm_readv(addr, &var, 8);
return var;
}

inline void getname(uintptr_t gname, int oid, char name[])
{
int page = oid / 0x4000;
int index = oid % 0x4000;
uintptr_t pa = getPtr64(gname + page * 8);
uintptr_t in = getPtr64(pa + index * 8);
vm_readv(in + 0xE, name, 32);
}

inline void writefloat(unsigned long addr, float data)
{
vm_writev(addr, &data, 4);
}

inline int getPID(const char *packageName)
{
int id = getProcessID(packageName);
if (id > 0) {
pid = id;
if (Mem().ready()) Mem().set_pid(id);
}
return id;
}

inline unsigned long get_module_base(int target_pid, const char *module_name)
{
if (target_pid > 0 && Mem().ready()) {
// 临时切 pid 查询
pid_t old = Mem().pid;
Mem().set_pid(target_pid);
uintptr_t b = Mem().module_base(module_name);
Mem().set_pid(old > 0 ? old : target_pid);
if (b) return (unsigned long)b;
}
return (unsigned long)MemBackend::maps_module_base(target_pid, module_name);
}

// module_index=1, bss_index=1 => libxxx.so:bss[1]
inline long getbss(const char *name, int target_pid = -1, int module_index = 1, int bss_index = 1)
{
    int p = (target_pid > 0) ? target_pid : pid;
    if (p <= 0 || !name) return 0;

    if (Mem().ready() && Mem().kind == MemBackendKind::Twt && bss_index == 1) {
        pid_t old = Mem().pid;
        Mem().set_pid(p);
        uintptr_t b = Mem().twt.module_bss(name);
        Mem().set_pid(old > 0 ? old : p);
        if (b) return (long)b;
    }

    return (long)MemBackend::maps_module_bss(p, name, module_index, bss_index);
}

inline int isapkrunning(PACKAGENAME * bm)
{
return getProcessID(bm) > 0 ? 1 : 0;
}

inline void getUTF8(UTF8 * buf, unsigned long namepy)
{
UTF16 buf16[16] = { 0 };
vm_readv(namepy, buf16, 28);
UTF16 *pTempUTF16 = buf16;
UTF8 *pTempUTF8 = buf;
UTF8 *pUTF8End = pTempUTF8 + 32;
while (pTempUTF16 < buf16 + 14)
{
if (*pTempUTF16 <= 0x007F && pTempUTF8 + 1 < pUTF8End)
{
*pTempUTF8++ = (UTF8) * pTempUTF16;
}
else if (*pTempUTF16 >= 0x0080 && *pTempUTF16 <= 0x07FF && pTempUTF8 + 2 < pUTF8End)
{
*pTempUTF8++ = (*pTempUTF16 >> 6) | 0xC0;
*pTempUTF8++ = (*pTempUTF16 & 0x3F) | 0x80;
}
else if (*pTempUTF16 >= 0x0800 && *pTempUTF16 <= 0xFFFF && pTempUTF8 + 3 < pUTF8End)
{
*pTempUTF8++ = (*pTempUTF16 >> 12) | 0xE0;
*pTempUTF8++ = ((*pTempUTF16 >> 6) & 0x3F) | 0x80;
*pTempUTF8++ = (*pTempUTF16 & 0x3F) | 0x80;
}
else
{
break;
}
pTempUTF16++;
}
}

// 计算骨骼
struct Vector2A
{
	float X;
	float Y;

	  Vector2A()
	{
		this->X = 0;
		this->Y = 0;
	}

	Vector2A(float x, float y)
	{
		this->X = x;
		this->Y = y;
	}
};

struct D2DVector
{
	float X;
	float Y;
};

struct D3DVector
{
	float X;
	float Y;
	float Z;
};

struct Vector3A
{
	float X;
	float Y;
	float Z;

	  Vector3A()
	{
		this->X = 0;
		this->Y = 0;
		this->Z = 0;
	}

	Vector3A(float x, float y, float z)
	{
		this->X = x;
		this->Y = y;
		this->Z = z;
	}

};



struct FMatrix
{
	float M[4][4];
};

class FRotator
{
public:
    FRotator() :Pitch(0.f), Yaw(0.f), Roll(0.f) {

    }
    FRotator(float _Pitch, float _Yaw, float _Roll) : Pitch(_Pitch), Yaw(_Yaw), Roll(_Roll)
    {

    }
    ~FRotator()
    {

    }
    float Pitch;
    float Yaw;
    float Roll;
    inline FRotator Clamp()
    {

        if (Pitch > 180)
        {
            Pitch -= 360;
        }
        else
        {
            if (Pitch < -180)
            {
                Pitch += 360;
            }
        }
        if (Yaw > 180)
        {
            Yaw -= 360;
        }
        else {
            if (Yaw < -180)
            {
                Yaw += 360;
            }
        }
        if (Pitch > 89)
        {
            Pitch = 89;
        }
        if (Pitch < -89)
        {
            Pitch = -89;
        }
        while (Yaw < 180)
        {
            Yaw += 360;
        }
        while (Yaw > 180)
        {
            Yaw -= 360;
        }
        Roll = 0;
        return FRotator(Pitch, Yaw, Roll);
    }
    inline float Length()
    {
        return sqrtf(Pitch * Pitch + Yaw * Yaw + Roll * Roll);
    }
    FRotator operator+(FRotator v) {
        return FRotator(Pitch + v.Pitch, Yaw + v.Yaw, Roll + v.Roll);
    }
    FRotator operator-(FRotator v) {
        return FRotator(Pitch - v.Pitch, Yaw - v.Yaw, Roll - v.Roll);
    }
};

struct Quat
{
	float X;
	float Y;
	float Z;
	float W;
};


struct FTransform
{
	Quat Rotation;
	Vector3A Translation;
	//float chunk;
	Vector3A Scale3D;
};


inline float get_3D_Distance(float Self_x, float Self_y, float Self_z, float Object_x, float Object_y,
					  float Object_z)
{
	float x, y, z;
	x = Self_x - Object_x;
	y = Self_y - Object_y;
	z = Self_z - Object_z;
	// 求平方根
	return (float)(sqrt(x * x + y * y + z * z));
}

// 计算旋转坐标
inline Vector2A rotateCoord(float angle, float objRadar_x, float objRadar_y)
{
	Vector2A radarCoordinate;
	float s = sin(angle * PI / 180);
	float c = cos(angle * PI / 180);
	radarCoordinate.X = objRadar_x * c + objRadar_y * s;
	radarCoordinate.Y = -objRadar_x * s + objRadar_y * c;
	return radarCoordinate;
}

inline Vector2A WorldToScreen(Vector3A obj, float matrix[16], float ViewW)
{
	float x =
		px + (matrix[0] * obj.X + matrix[4] * obj.Y + matrix[8] * obj.Z + matrix[12]) / ViewW * px;
	float y =
		py - (matrix[1] * obj.X + matrix[5] * obj.Y + matrix[9] * obj.Z + matrix[13]) / ViewW * py;

	return Vector2A(x, y);
}

inline Vector3A MarixToVector(FMatrix matrix)
{
	return Vector3A(matrix.M[3][0], matrix.M[3][1], matrix.M[3][2]);
}

inline FMatrix MatrixMulti(FMatrix m1, FMatrix m2)
{
	FMatrix matrix = FMatrix();
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			for (int k = 0; k < 4; k++)
			{
				matrix.M[i][j] += m1.M[i][k] * m2.M[k][j];
			}
		}
	}
	return matrix;
}

inline FMatrix TransformToMatrix(FTransform transform)
{
	FMatrix matrix;
	matrix.M[3][0] = transform.Translation.X;
	matrix.M[3][1] = transform.Translation.Y;
	matrix.M[3][2] = transform.Translation.Z;
	float x2 = transform.Rotation.X + transform.Rotation.X;
	float y2 = transform.Rotation.Y + transform.Rotation.Y;
	float z2 = transform.Rotation.Z + transform.Rotation.Z;
	float xx2 = transform.Rotation.X * x2;
	float yy2 = transform.Rotation.Y * y2;
	float zz2 = transform.Rotation.Z * z2;
	matrix.M[0][0] = (1 - (yy2 + zz2)) * transform.Scale3D.X;
	matrix.M[1][1] = (1 - (xx2 + zz2)) * transform.Scale3D.Y;
	matrix.M[2][2] = (1 - (xx2 + yy2)) * transform.Scale3D.Z;
	float yz2 = transform.Rotation.Y * z2;
	float wx2 = transform.Rotation.W * x2;
	matrix.M[2][1] = (yz2 - wx2) * transform.Scale3D.Z;
	matrix.M[1][2] = (yz2 + wx2) * transform.Scale3D.Y;
	float xy2 = transform.Rotation.X * y2;
	float wz2 = transform.Rotation.W * z2;
	matrix.M[1][0] = (xy2 - wz2) * transform.Scale3D.Y;
	matrix.M[0][1] = (xy2 + wz2) * transform.Scale3D.X;
	float xz2 = transform.Rotation.X * z2;
	float wy2 = transform.Rotation.W * y2;
	matrix.M[2][0] = (xz2 + wy2) * transform.Scale3D.Z;
	matrix.M[0][2] = (xz2 - wy2) * transform.Scale3D.X;
	matrix.M[0][3] = 0;
	matrix.M[1][3] = 0;
	matrix.M[2][3] = 0;
	matrix.M[3][3] = 1;
	return matrix;
}

inline FTransform getBone(unsigned long addr)
{
	FTransform transform;
	vm_readv(addr, &transform, 4 * 11);
	return transform;
}



struct D3DXMATRIX
{
	float _11;
	float _12;
	float _13;
	float _14;
	float _21;
	float _22;
	float _23;
	float _24;
	float _31;
	float _32;
	float _33;
	float _34;
	float _41;
	float _42;
	float _43;
	float _44;
};

struct D3DXVECTOR4
{
	float X;
	float Y;
	float Z;
	float W;
};

struct FTransform1
{
	D3DXVECTOR4 Rotation;
	D3DVector Translation;
	D3DVector Scale3D;
};

inline D3DXMATRIX ToMatrixWithScale(D3DXVECTOR4 Rotation, D3DVector Translation, D3DVector Scale3D)
{
	D3DXMATRIX M;
	float X2, Y2, Z2, xX2, Yy2, Zz2, Zy2, Wx2, Xy2, Wz2, Zx2, Wy2;
	M._41 = Translation.X;
	M._42 = Translation.Y;
	M._43 = Translation.Z;
	X2 = Rotation.X + Rotation.X;
	Y2 = Rotation.Y + Rotation.Y;
	Z2 = Rotation.Z + Rotation.Z;
	xX2 = Rotation.X * X2;
	Yy2 = Rotation.Y * Y2;
	Zz2 = Rotation.Z * Z2;
	M._11 = (1 - (Yy2 + Zz2)) * Scale3D.X;
	M._22 = (1 - (xX2 + Zz2)) * Scale3D.Y;
	M._33 = (1 - (xX2 + Yy2)) * Scale3D.Z;
	Zy2 = Rotation.Y * Z2;
	Wx2 = Rotation.W * X2;
	M._32 = (Zy2 - Wx2) * Scale3D.Z;
	M._23 = (Zy2 + Wx2) * Scale3D.Y;
	Xy2 = Rotation.X * Y2;
	Wz2 = Rotation.W * Z2;
	M._21 = (Xy2 - Wz2) * Scale3D.Y;
	M._12 = (Xy2 + Wz2) * Scale3D.X;
	Zx2 = Rotation.X * Z2;
	Wy2 = Rotation.W * Y2;
	M._31 = (Zx2 + Wy2) * Scale3D.Z;
	M._13 = (Zx2 - Wy2) * Scale3D.X;
	M._14 = 0;
	M._24 = 0;
	M._34 = 0;
	M._44 = 1;
	return M;
}

inline FTransform1 ReadFTransform(long int address)
{
	FTransform1 Result;
	Result.Rotation.X = getFloat(address);	// Rotation_X 
	Result.Rotation.Y = getFloat(address + 4);	// Rotation_y
	Result.Rotation.Z = getFloat(address + 8);	// Rotation_z
	Result.Rotation.W = getFloat(address + 12);	// Rotation_w
	Result.Translation.X = getFloat(address + 16);	// /Translation_X
	Result.Translation.Y = getFloat(address + 20);	// Translation_y
	Result.Translation.Z = getFloat(address + 24);	// Translation_z
	Result.Scale3D.X = getFloat(address + 32);;	// Scale_X
	Result.Scale3D.Y = getFloat(address + 36);;	// Scale_y
	Result.Scale3D.Z = getFloat(address + 40);;	// Scale_z
	return Result;
}

// 获取骨骼3d坐标
inline D3DVector D3dMatrixMultiply(D3DXMATRIX bonematrix, D3DXMATRIX actormatrix)
{
	D3DVector result;
	result.X =
		bonematrix._41 * actormatrix._11 + bonematrix._42 * actormatrix._21 +
		bonematrix._43 * actormatrix._31 + bonematrix._44 * actormatrix._41;
	result.Y =
		bonematrix._41 * actormatrix._12 + bonematrix._42 * actormatrix._22 +
		bonematrix._43 * actormatrix._32 + bonematrix._44 * actormatrix._42;
	result.Z =
		bonematrix._41 * actormatrix._13 + bonematrix._42 * actormatrix._23 +
		bonematrix._43 * actormatrix._33 + bonematrix._44 * actormatrix._43;
	return result;
}

inline D3DVector getBoneXYZ(long int humanAddr, long int boneAddr, int Part)
{
	// 获取Bone数据
	FTransform1 Bone = ReadFTransform(boneAddr + Part * 48);
	// 获取Actor数据
	FTransform1 Actor = ReadFTransform(humanAddr);
	D3DXMATRIX Bone_Matrix = ToMatrixWithScale(Bone.Rotation, Bone.Translation, Bone.Scale3D);
	D3DXMATRIX Component_ToWorld_Matrix =
		ToMatrixWithScale(Actor.Rotation, Actor.Translation, Actor.Scale3D);
	D3DVector result = D3dMatrixMultiply(Bone_Matrix, Component_ToWorld_Matrix);
	return result;
}


inline double ArcToAngle(double angle)
{
    return angle * (double)57.29577951308;
}

inline Vector3A CalcAngle(Vector3A D, Vector3A W)
{
    float x = W.X - D.X;
    float y = W.Y - D.Y;
    float z = W.Z - D.Z;
    Vector3A angle = Vector3A(ArcToAngle(atan2(y, x)), ArcToAngle(atan2(z, sqrt(x * x + y * y))), 0.0f);
    return angle;
}

#endif
