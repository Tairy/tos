//
// Created by tairy on 2020/11/6.
//
#include "memory.h"
#include "lib.h"

unsigned long page_init(struct Page *page, unsigned long flags) {
    if (!page->attribute) {
        *(memory_management_struct.bits_map + ((page->PHY_address >> PAGE_2M_SHIFT) >> 6)) |=
                1UL << (page->PHY_address >> PAGE_2M_SHIFT) % 64;
        page->attribute = flags;
        page->reference_count++;
        page->zone_struct->page_using_count++;
        page->zone_struct->page_free_count--;
        page->zone_struct->total_pages_link++;
    } else if ((page->attribute & PG_Referenced) || (page->attribute & PG_K_Share_To_U) || (flags & PG_Referenced) ||
               (flags & PG_K_Share_To_U)) {
        page->attribute |= flags;
        page->reference_count++;
        page->zone_struct->total_pages_link++;
    } else {
        *(memory_management_struct.bits_map + ((page->PHY_address >> PAGE_2M_SHIFT) >> 6)) |=
                1UL << (page->PHY_address >> PAGE_2M_SHIFT) % 64;
        page->attribute |= flags;
    }

    return 0;
}

void init_memory() {
    int i, j;

    // 显示物理内存分布信息
    unsigned long TotalMem = 0;
    struct E820 *p = NULL;

    color_printk(BLUE, BLACK,
                 "Display Address MAP,Type(1:RAM,2:ROM or Reserved,3:ACPI Reclaim Memory,4:ACPI NVS Memory,Others:undefined)\n");
    p = (struct E820 *) 0xffff800000007e00; // BIOS 通过 INT15 获取到的物理内存信息存储位置

    for (i = 0; i < 32; i++) {
        color_printk(ORANGE, BLACK, "Address:%#018lx\tLength:%#018lx\tType:%#010x\n", p->address, p->length, p->type);
        unsigned long tmp = 0;
        if (p->type == 1) {
            TotalMem += p->length;
        }

        memory_management_struct.e820[i].address += p->address; // 这里的 += 与 = 没区别
        memory_management_struct.e820[i].length += p->length;
        memory_management_struct.e820[i].type = p->type;
        memory_management_struct.e820_length = i;

        p++;

        //截断并剔除 E820 数组中的脏数据
        if (p->type > 4 || p->length == 0 || p->type < 1) {
            break;
        }
    }

    color_printk(ORANGE, BLACK, "OS Can Used Total RAM:%#018lx\n", TotalMem);

    // 对 e820 结构体数组中的可用物理内存段进行 2 MB 物理内存对齐，并统计出物理页的总量
    TotalMem = 0;
    for (int i = 0; i < memory_management_struct.e820_length; i++) {
        unsigned long start, end;

        // 可用物理内存 type = 1
        if (memory_management_struct.e820[i].type != 1) {
            continue;
        }

        // min 2M
        start = PAGE_2M_ALIGN(memory_management_struct.e820[i].address);
        end = ((memory_management_struct.e820[i].address + memory_management_struct.e820[i].length) >> PAGE_2M_SHIFT)
                << PAGE_2M_SHIFT;
        if (end <= start) {
            continue;
        }

        // 总的物理页数
        TotalMem += (end - start) >> PAGE_2M_SHIFT;
    }

    color_printk(ORANGE, BLACK, "OS Can Used Total 2M PAGEs:%#010x=%010d\n", TotalMem, TotalMem);

    // 总的物理内存 单位 bit
    TotalMem = memory_management_struct.e820[memory_management_struct.e820_length].address +
               memory_management_struct.e820[memory_management_struct.e820_length].length;

    // bits map construction init
    // bits_map 是映射位图的指针，指向内核程序结束地址 end_brk 的 4KB 上边界的对其处，
    // 此举是为了保留一小段隔离空间，以防止误操作损坏其他空间数据
    // 紧接着将 bit_maps 空间全部置位，以标注非内存页（内存空洞和 ROM 空间）已被使用，
    // 随后再通过程序将映射位图中的可用物理内存复位
    memory_management_struct.bits_map = (unsigned long *) ((memory_management_struct.end_brk + PAGE_4k_SIZE - 1) &
                                                           PAGE_4K_MASK);
    memory_management_struct.bits_size = TotalMem >> PAGE_2M_SHIFT;
    memory_management_struct.bits_length =
            (((unsigned long) (TotalMem >> PAGE_2M_SHIFT) + sizeof(long) * 8 - 1) / 8) & (~(sizeof(long) - 1));
    memset(memory_management_struct.bits_map, 0xff, memory_management_struct.bits_length); // init bits map memory

    // page construction init
    // struct_page 结构体存储位于 bits_map 之后
    // 数组的元素数量为物理地址空间可分页数
    // struct_page 结构体数组全部清零已被后续初始化使用
    memory_management_struct.pages_struct = (struct Page *) (
            ((unsigned long) memory_management_struct.bits_map + memory_management_struct.bits_length + PAGE_4k_SIZE -
             1) & PAGE_4K_MASK);
    memory_management_struct.pages_size = TotalMem >> PAGE_2M_SHIFT;
    memory_management_struct.pages_length =
            ((TotalMem >> PAGE_2M_SHIFT) * sizeof(struct Page) + sizeof(long) - 1) & (~(sizeof(long) - 1));

    memset(memory_management_struct.pages_struct, 0x00, memory_management_struct.pages_length);

    // zones construction init
    // 目前无法计算出 struct_zone 结构体数组的个数，只能将 zones_size 成员变量赋值为 0
    // 而将 zones_length 成员变量暂且按照 5 个 struct_zone 结构体来计算
    memory_management_struct.zones_struct = (struct Zone *) (
            ((unsigned long) memory_management_struct.pages_struct + memory_management_struct.pages_length +
             PAGE_4k_SIZE - 1) & PAGE_4K_MASK);
    memory_management_struct.zones_size = 0;
    memory_management_struct.zones_length = (5 * sizeof(struct Zone) + sizeof(long) - 1) & (~(sizeof(long) - 1));

    memset(memory_management_struct.zones_struct, 0x00, memory_management_struct.zones_length);

    // 上述步骤创建了存储 memory_management_struct 成员变量的存储空间，后续遍历 e820 数组初始化这些成员变量
    for (int i = 0; i < memory_management_struct.e820_length; i++) {
        unsigned long start, end;
        struct Zone *z;
        struct Page *p;
        unsigned long *b;

        if (memory_management_struct.e820[i].type != 1) {
            continue;
        }

        start = PAGE_2M_ALIGN(memory_management_struct.e820[i].address);
        end = ((memory_management_struct.e820[i].address + memory_management_struct.e820[i].length) >> PAGE_2M_SHIFT)
                << PAGE_2M_SHIFT;
        if (end <= start) {
            continue;
        }

        // zone init
        z = memory_management_struct.zones_struct + memory_management_struct.zones_size;
        memory_management_struct.zones_size++;

        z->zone_start_address = start;
        z->zone_end_address = end;
        z->zone_length = start - end;

        z->page_using_count = 0;
        z->page_free_count = (end - start) >> PAGE_2M_SHIFT;
        z->total_pages_link = 0;

        z->attribute = 0;
        z->GMD_struct = &memory_management_struct;
        z->pages_length = (end - start) >> PAGE_2M_SHIFT;
        z->pages_group = (struct Page *) (memory_management_struct.pages_struct + (start >> PAGE_2M_SHIFT));

        // page init
        p = z->pages_group;
        for (j = 0; j < z->pages_length; j++, p++) {
            p->zone_struct = z;
            p->PHY_address = start + PAGE_2M_SIZE * j;
            p->attribute = 0;

            p->reference_count = 0;
            p->age = 0;

            // 把当前 struct_page 结构体所代表的物理地址转换成 bits_map 映射位图中对应的位
            *(memory_management_struct.bits_map + ((p->PHY_address >> PAGE_2M_SHIFT) >> 6)) ^=
                    1UL << (p->PHY_address >> PAGE_2M_SHIFT) % 64;
        }
    }

    // init address 0 to page struct 0
    // 对 0 ~ 2MB 的物理内存进行特殊的初始化
    memory_management_struct.pages_struct->zone_struct = memory_management_struct.zones_struct;
    memory_management_struct.pages_struct->PHY_address = 0UL;
    memory_management_struct.pages_struct->attribute = 0;
    memory_management_struct.pages_struct->reference_count = 0;
    memory_management_struct.pages_struct->age = 0;
    memory_management_struct.zones_length =
            (memory_management_struct.zones_size * sizeof(struct Zone) + sizeof(long) - 1) & (~sizeof(long) - 1);

    color_printk(ORANGE, BLACK, "bits_map:%#018lx,bits_size:%#018lx,bits_length:%#018lx\n",
                 memory_management_struct.bits_map, memory_management_struct.bits_size,
                 memory_management_struct.bits_length);

    color_printk(ORANGE, BLACK, "pages_struct:%#018lx,pages_size:%#018lx,pages_length:%#018lx\n",
                 memory_management_struct.pages_struct, memory_management_struct.pages_size,
                 memory_management_struct.pages_length);

    color_printk(ORANGE, BLACK, "zones_struct:%#018lx,zones_size:%#018lx,zones_length:%#018lx\n",
                 memory_management_struct.zones_struct, memory_management_struct.zones_size,
                 memory_management_struct.zones_length);

    ZONE_DMA_INDEX = 0;
    ZONE_NORMAL_INDEX = 0;

    for (int i = 0; i < memory_management_struct.zones_size; i++) {
        struct Zone *z = memory_management_struct.zones_struct + i;
        color_printk(ORANGE, BLACK,
                     "zone_start_address:%#018lx,zone_end_address:%#018lx,zone_length:%#018lx,pages_group:%#018lx,pages_length:%#018lx\n",
                     z->zone_start_address, z->zone_end_address, z->zone_length, z->pages_group, z->pages_length);
        if (z->zone_start_address == 0x100000000) {
            ZONE_UNMAPPED_INDEX = i;
        }
    }

    memory_management_struct.end_of_struct = (unsigned long) ((unsigned long) memory_management_struct.zones_struct +
                                                              memory_management_struct.zones_length +
                                                              sizeof(long) * 32) & (~(sizeof(long) - 1));

    color_printk(ORANGE, BLACK,
                 "start_code:%#018lx,end_code:%#018lx,end_data:%#018lx,end_brk:%#018lx,end_of_struct:%#018lx\n",
                 memory_management_struct.start_code, memory_management_struct.end_code,
                 memory_management_struct.end_data, memory_management_struct.end_brk,
                 memory_management_struct.end_of_struct);

    i = Virt_To_Phy(memory_management_struct.end_of_struct) >> PAGE_2M_SHIFT;

    for (int j = 0; j < i; ++j) {
        page_init(memory_management_struct.pages_struct + j, PG_PTable_Mapped | PG_Kernel_Init | PG_Active | PG_Kernel);
    }

    Global_CR3 = Get_gdt();

    color_printk(INDIGO, BLACK, "Global_CR3\t:%#018lx\n", Global_CR3);
    color_printk(INDIGO, BLACK, "*Global_CR3\t:%#018lx\n", *Phy_To_Virt(Global_CR3) & (~0xff));
    color_printk(PURPLE, BLACK, "**Global_CR3\t:%#018lx\n", *Phy_To_Virt(*Phy_To_Virt(Global_CR3) & (~0xff)) & (~0xff));

    for (i = 0; i < 10; i++) {
        *(Phy_To_Virt(Global_CR3) + i) = 0UL;
    }

    flush_tlb();
}

struct Page *alloc_pages(int zone_select, int number, unsigned long page_flags) {
    int i;
    unsigned long page = 0;

    int zone_start = 0;
    int zone_end = 0;

    switch (zone_select) {
        case ZONE_DMA:
            zone_start = 0;
            zone_end = ZONE_DMA_INDEX;
            break;
        case ZONE_NORMAL:
            zone_start = ZONE_DMA_INDEX;
            zone_end = ZONE_NORMAL_INDEX;
            break;
        case ZONE_UNMAPPED:
            zone_start = ZONE_UNMAPPED_INDEX;
            zone_end = memory_management_struct.zones_size - 1;
            break;
        default:
            color_printk(RED, BLACK, "alloc_pages error zone_select index\n");
            return NULL;
    }

    for (int i = zone_start; i <= zone_end; i++) {
        struct Zone *z;
        unsigned long j;
        unsigned long start, end, length;
        unsigned long tmp;

        if ((memory_management_struct.zones_struct + i)->page_free_count < number) {
            continue;
        }

        z = memory_management_struct.zones_struct + i;
        start = z->zone_start_address >> PAGE_2M_SHIFT; // 这样移位可以计算出 address 在 bit_map 的索引
        end = z->zone_end_address >> PAGE_2M_SHIFT;
        length = z->zone_length >> PAGE_2M_SHIFT;

        tmp = 64 - start % 64;

        for (j = start; j <= end; j += j % 64 ? tmp : 64) {
            unsigned long *p = memory_management_struct.bits_map + (j >> 6);
            unsigned long shift = j % 64;
            unsigned long k;

            for (int k = shift; k < 64 - shift; k++) {
                if (!(((*p >> k) | (*(p + 1) << (64 - k))) &
                      (number == 64 ? 0xffffffffffffffffUL : ((1UL << number) - 1)))) {
                    unsigned long l;
                    page = j + k - 1;
                    for (l = 0; l < number; l++) {
                        struct Page *x = memory_management_struct.pages_struct + page + l;
                        page_init(x, page_flags);
                    }
                    goto find_free_pages;
                }
            }
        }

        return NULL;
        find_free_pages:
        return (struct Page *) (memory_management_struct.pages_struct + page);
    }
}