import requests
from bs4 import BeautifulSoup

desired_calories = 1900
desired_daily_price = 500
max_dessert_calories = 500

class Dish:
    def __lt__(self, other):
        return self.factor < other.factor
    name: str
    category: str
    weight: float
    calories: float
    price: float
    factor: float


def generate_courses():
    # Load paige and create soup
    mealy_url = 'https://www.mealty.ru/catalog/'
    html_text = requests.get(mealy_url).text
    soup = BeautifulSoup(html_text, 'html.parser')

    # Some data used to cut out courses we are not interested in
    categories_of_interest = list()
    categories_of_interest.append('breakfast')
    categories_of_interest.append('salad')
    categories_of_interest.append('soup')
    categories_of_interest.append('main_dish')
    categories_of_interest.append('drink')
    categories_of_interest.append('bread')
    exceptions = ['Хлеб', 'Тартин', 'Булочка']
    categories = soup.find_all('div', {'class': ['landingItem'], 'id': categories_of_interest})

    meals = []
    controls = []
    for category in categories:
        meals.extend(category.find_all('div', class_='meal-card'))
        controls.extend(category.find_all('div', class_='meal-card__controls'))

    dishes = []

    for meal, control in zip(meals, controls):
        if control.find('div', class_="meal-card__buttons out-of-stock-hide hidden"):
            continue
        main_name = meal.find('div', class_='meal-card__name').text
        found = False
        for exception in exceptions:
            if exception in main_name or exception.lower() in main_name:
                found = True
                continue
        if found:
            break
        second_name = meal.find('div', class_='meal-card__name-note').text

        dish = Dish()
        dish.name = meal.find('div', class_='meal-card__name').text
        if len(second_name) > 0:
            dish.name += ' ' + second_name
        dish.weight = float(meal.find('div', class_='meal-card__weight').text)
        dish.calories = float(meal.find('div', class_='meal-card__calories').text)
        dish.price = float(control.find('span', class_='basket__footer-total-count green').text)
        dish.factor = dish.weight * dish.calories / dish.price / 100.0
        dishes.append(dish)

    dishes.sort()
    for dish in dishes:
        print(dish.name + ' : ' + str(dish.factor))


# Press the green button in the gutter to run the script.
if __name__ == '__main__':
    generate_courses()

# See PyCharm help at https://www.jetbrains.com/help/pycharm/
