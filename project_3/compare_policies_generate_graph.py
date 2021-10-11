import matplotlib.pyplot as plt
from matplotlib.pyplot import figure

strat = ["custom", "rand", "fifo"]
algo = ["sort", "focus", "scan"]

pf = []
dw = []
x = [i for i in range(2, 101)]

for i, s in enumerate(strat):
    pf.append([])
    dw.append([])
    for j, a in enumerate(algo):
        pf[i].append([])
        dw[i].append([])
        with open("csv-files/{}-{}.csv".format(s, a), "r") as fd:
            count = 0
            for line in fd:
                line = line.strip("\n").strip("\r").split(",")
                count += 1
                if len(line) == 0 or count == 1:
                    continue
                _frame = int(line[1])
                if _frame == 1:
                    continue
                _page_fault = int(line[2])
                _disk_writes = int(line[4])
                
                pf[i][j].append(_page_fault)
                dw[i][j].append(_disk_writes)


fig = figure(num=None, figsize=(30,16), dpi=80, facecolor='w', edgecolor='k')
fig.suptitle('Using pagefaults and diskwrites to compare paging strategy for different access patterns', fontsize=18)

ax1 = fig.add_subplot(231)
ax2 = fig.add_subplot(232)
ax3 = fig.add_subplot(233)
ax4 = fig.add_subplot(234)
ax5 = fig.add_subplot(235)
ax6 = fig.add_subplot(236)

ax1.title.set_text('Frames vs Page Fault - Sort')
ax2.title.set_text('Frames vs Page Fault - Focus')
ax3.title.set_text('Frames vs Page Fault - Scan')
ax4.title.set_text('Frames vs Disk Writes - Sort')
ax5.title.set_text('Frames vs Disk Writes - Focus')
ax6.title.set_text('Frames vs Disk Writes - Scan')

count = 1
for j in range(0, 3):
    plt.subplot(2, 3, count)
    count += 1
    plt.xlabel('Frames')#, fontsize=16)
    plt.ylabel('Page faults')#, fontsize=16)
    # plt.plot([0,1], [0,1], 'bo-', label='Random classifier')
    plt.plot(x, pf[0][j], 'ro-', label = "custom", alpha=.5, linewidth=4.5)
    plt.plot(x, pf[1][j], 'go-', label = "rand")
    plt.plot(x, pf[2][j], 'bo-', label = "fifo", alpha=.5)
    plt.legend(loc="upper left")
    
for j in range(0, 3):
    plt.subplot(2, 3, count)
    count += 1
    plt.xlabel('Frames')#, fontsize=16)
    plt.ylabel('Disk Writes')#, fontsize=16)
    # plt.plot([0,1], [0,1], 'bo-', label='Random classifier')
    plt.plot(x, dw[0][j], 'ro-', label = "custom", alpha=.5, linewidth=4.5)
    plt.plot(x, dw[1][j], 'go-', label = "rand")
    plt.plot(x, dw[2][j], 'bo-', label = "fifo", alpha=.5)
    plt.legend(loc="upper left")

fig.savefig('Compare_policies.png')