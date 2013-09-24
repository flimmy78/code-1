#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/unistd.h>
#include <linux/sched.h>
unsigned long *sys_call_table = NULL;
static unsigned int origcr0 = 0;
asmlinkage int (*orig_mkdir)(const char*,int mode);
unsigned long orig_mkdir_address  = 0;
struct idt_tag
{
    unsigned short offset_low,segment_select;
    unsigned char reserved,flags;
    unsigned short offset_high;
};
unsigned int clear_and_return_origCR0(void)
{
    unsigned int cr0 = 0;
    unsigned int ret = -1;
    __asm__ __volatile__ (
        "movl %%cr0,%%eax"
        :"=a"(cr0)
    );
    ret = cr0;
    cr0 = cr0 & 0xfffeffff;
    __asm__ __volatile__ (
        "movl %%eax,%%cr0"
        ::"a"(cr0)
    );
    return ret;
}
void setbackCR0(unsigned int pCR0)
{
    __asm__ __volatile__ (
        "movl %%eax,%%cr0"
        ::"a"(pCR0)
    );
}
static unsigned long getSyscallTable(void)
{
    unsigned char idtr[6],*shell,*sort;
    struct idt_tag *idt;
    unsigned long system_call,sct;
    unsigned short offset_low,offset_high;
    char *p;
    int i;
    __asm__("sidt %0":"=m"(idtr));
    idt = (struct idt_tag*)((*(unsigned long*)&idtr[2]) + 8 * 0x80);
    offset_low = idt->offset_low;
    offset_high = idt->offset_high;
    system_call = (offset_high)<<16 | offset_low;    
    shell = (char*)system_call;
    sort = "\xff\x14\x85";
 
    for(i = 0;i < 100-2;i++)
        if(shell [ i ] == sort[0] && shell[i+1] == sort[1] && shell[i+2] == sort[2])
            break;
    p = &shell [ i ] + 3;
    sct = *(unsigned long*)p;
    return sct;    
}
 
asmlinkage int hack_mkdir(const char* pathname,int mode)
{
    printk("called sys_mkdir,but gain nothing,hei hei!!\n");
    return 0;
}
 
static int __init myinit(void)
{
    printk("<1>: ********************Just enter the myinit()...\n");
    sys_call_table = (unsigned long*)getSyscallTable();
    printk("<1>: ********************Just after calling getSyscallTable()...\n");
    orig_mkdir_address = sys_call_table[__NR_mkdir];
    printk("<1>: ***************Just get the orig_mkdir = %lux\n",orig_mkdir_address);
 
    origcr0 = clear_and_return_origCR0();
    sys_call_table[__NR_mkdir] = (unsigned long)hack_mkdir;
    printk("<1>: ***************Just change the syscall....\n");
    return 0;
}
 
static void __exit myexit(void)
{
    sys_call_table[__NR_mkdir] = orig_mkdir_address;
    setbackCR0(origcr0);
}
 
module_init(myinit);
module_exit(myexit);
