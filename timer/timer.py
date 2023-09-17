import time
import machine
from pimoroni import Button
from picographics import PicoGraphics, DISPLAY_PICO_DISPLAY, PEN_P4

# We're only using a few colours so we can use a 4 bit/16 colour palette and save RAM
display = PicoGraphics(display=DISPLAY_PICO_DISPLAY, pen_type=PEN_P4, rotate=0)

display.set_font("bitmap8")

button_a = Button(12)
button_b = Button(13)
button_x = Button(14)
button_y = Button(15)

WHITE = display.create_pen(255, 255, 255)
BLACK = display.create_pen(0, 0, 0)
BG = display.create_pen(40, 40, 40)

TIME_SECONDS = 1694879089


def clear():
    display.set_pen(BLACK)
    display.clear()
    display.update()


def format_time(seconds):
    days, seconds = divmod(seconds, 86400)
    hours, seconds = divmod(seconds, 3600)
    minutes, seconds = divmod(seconds, 60)
    return days, hours, minutes, seconds


def getTimeDifference(timeSeconds):
    currentTimestampSeconds = time.time()
    timeDifference = currentTimestampSeconds - timeSeconds
    return timeDifference


def getCurrentTime():
    current_time = time.localtime(time.time())
    year, month, mday, hour, minute, second, _, _ = current_time
    formatted_time = "{:02d}/{:02d}/{:04d} {:02d}:{:02d}:{:02d}".format(
        month, mday, year, hour, minute, second)
    return formatted_time


def updateCurrentTime():
    weekday = day_of_week(current_year, current_month, current_day)
    machine.RTC().datetime((current_year, current_month, current_day,
                            day_of_week, current_hour, current_minute, current_second, 0))


def day_of_week(year, month, day):
    if month < 3:
        month += 12
        year -= 1
    q = day
    m = month
    K = year % 100
    J = year // 100
    f = q + (13 * (m + 1)) // 5 + K + K // 4 + J // 4 - 2 * J
    day_of_week = f % 7
    return day_of_week


time_adjustment_mode = False
date_adjustment_mode = False
year_adjustment_mode = False

current_second = 0
current_hour = 14
current_minute = 12
current_day = 16
current_month = 9
current_year = 2023


def adjust_time():
    global current_hour, current_minute

    if button_a.read():
        current_hour = (current_hour + 1) % 24

    if button_b.read():
        current_minute = (current_minute + 1) % 60

    return "{:02d}:{:02d}".format(current_hour, current_minute)


def adjust_date():
    global current_day, current_month

    # Number of days in each month. Considering February has 28 days
    days_in_month = [0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]

    if button_a.read():
        current_day += 1
        if current_day > days_in_month[current_month]:
            current_day = 1
    if button_b.read():
        current_month = (current_month + 1) % 13
        if current_month == 0:  # month should be between 1 and 12
            current_month = 1
        # if current day exceeds the days in the incremented month
        if current_day > days_in_month[current_month]:
            current_day = days_in_month[current_month]

    return "{:02d}/{:02d}".format(current_month, current_day)


def adjust_year():
    global current_year

    if button_a.read():
        current_year += 1
    if button_b.read():
        current_year -= 1

    return "{:04d}".format(current_year)


while True:
    if time_adjustment_mode:
        current_time = adjust_time()
        display.set_pen(BG)
        display.clear()
        display.set_pen(WHITE)
        display.text("Adjusting Time", 10, 10, 240, 2)
        display.text(current_time, 10, 40, scale=3)
        if button_x.read():
            time_adjustment_mode = False
            updateCurrentTime()

    elif date_adjustment_mode:
        current_date = adjust_date()
        display.set_pen(BG)
        display.clear()
        display.set_pen(WHITE)
        display.text("Adjusting Date", 10, 10, 240, 2)
        display.text(current_date, 10, 40, scale=3)
        if button_x.read():
            date_adjustment_mode = False
            updateCurrentTime()
    elif year_adjustment_mode:
        new_year = adjust_year()
        display.set_pen(BG)
        display.clear()
        display.set_pen(WHITE)
        display.text("Adjusting Year", 10, 10, 240, 2)
        display.text(new_year, 10, 40, scale=3)
        if button_x.read():
            year_adjustment_mode = False
            updateCurrentTime()
    else:
        display.set_pen(BG)
        display.clear()

        display.set_pen(WHITE)
        current_time = getCurrentTime()

        timeDifference = getTimeDifference(TIME_SECONDS)
        days, hours, minutes, seconds = format_time(timeDifference)
        time_str = f"{days}d {hours}h {minutes}m {seconds}s"

        display.text(current_time, 10, 10, scale=2)
        display.text(time_str, 10, 40, scale=3)

        # Datetime edit modes
        if button_a.is_pressed:
            start_time = time.time()
            while button_a.is_pressed:
                time.sleep(0.1)
                if time.time() - start_time > 0:
                    print("time_adjustment_mode")
                    time_adjustment_mode = True

        if button_b.is_pressed:
            start_time = time.time()
            while button_b.is_pressed:
                if time.time() - start_time > 0:
                    print("date_adjustment_mode")
                    date_adjustment_mode = True

        if button_y.is_pressed:
            start_time = time.time()
            while button_y.is_pressed:
                if time.time() - start_time > 0:
                    print("year_adjustment_mode")
                    year_adjustment_mode = True

    display.update()
    time.sleep(0.01)
