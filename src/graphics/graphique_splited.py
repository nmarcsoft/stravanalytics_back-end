import sys

import matplotlib.pyplot as plt

def draw_graph():
    x_list = list(map(float, sys.argv[1].split(",")))
    pace_vma = list(map(float, sys.argv[2].split(",")))
    pace_ef = list(map(float, sys.argv[3].split(",")))

    for i in range(len(pace_vma)):
        plt.axvspan(x_list[i] - 0.50, x_list[i] + 0.50, facecolor = "#f77f77")

    for i in range(len(pace_ef)):
        plt.axvspan(x_list[len(pace_vma) + i] - 0.50, x_list[len(pace_vma) + i] + 0.50, facecolor = "#97b7fc")

    pace = pace_vma + pace_ef

    plt.plot(x_list, pace, marker='o')
    plt.xlabel('Numéro du footing')
    plt.ylabel('Allure (min/km)')
    plt.title('Allure par footing selon type')
    #plt.legend()
    plt.grid(True)
    plt.gca().invert_yaxis()
    plt.savefig('output/allure_par_footing_selon_type.png')

draw_graph()
