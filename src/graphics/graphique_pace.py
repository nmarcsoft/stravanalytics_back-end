import sys

def draw_pace(x_list, speed_list):
    import matplotlib.pyplot as plt
    x = x_list
    pace = speed_list
    plt.figure(figsize=(10,5))
    plt.plot(x, pace, marker='o')
    plt.xlabel('Numéro du footing')
    plt.ylabel('Allure (min/km)')
    plt.title('Allure par footing')
    plt.grid(True)
    plt.gca().invert_yaxis()
    plt.savefig('output/allure_par_footing.png')

    print("Graphique généré")

x_list = list(map(float, sys.argv[1].split(",")))
speed_list = list(map(float, sys.argv[2].split(",")))

draw_pace(x_list, speed_list)

'''
with open("output/debug_python", "w") as f:
    f.write(sys.argv[1])
    f.write(sys.argv[2])
'''
