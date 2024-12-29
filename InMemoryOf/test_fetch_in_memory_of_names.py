from time import sleep

from playwright.sync_api import Page

IDF_URL = "https://www.idf.il/%D7%A0%D7%95%D7%A4%D7%9C%D7%99%D7%9D/%D7%97%D7%9C%D7%9C%D7%99-%D7%94%D7%9E%D7%9C%D7%97%D7%9E%D7%94/"
BTL_URL = "https://laad.btl.gov.il/Web/He/HaravotBarzelWar/Default.aspx"


def test_idf_names(page: Page) -> None:
    names = []
    page_number = 1
    while True:
        page.goto(f"{IDF_URL}?page={page_number}")
        for element in page.query_selector_all(".solder-name"):
            first_child = element.query_selector("span")
            last_child = element.query_selector("span:last-child")
            names.append(
                element.inner_text()
                .replace(first_child.inner_text(), "")
                .replace(last_child.inner_text(), "")
                .strip() + "\n"
            )
        if len(page.query_selector_all("li.page-item.page-item-next > a")) == 0:
            break
        page_number += 1
        print(f"finish running page number: {page_number}")
    print(f"found {len(names)} results :(")
    with open("idf.txt", "w+") as file:
        file.writelines(names)


def test_btl_names(page: Page) -> None:
    page.goto(BTL_URL)
    names = []
    while True:
        for element in page.query_selector_all("li > dl > dd.title > a > span:first-child"):
            names.append(f"{element.inner_text()}\n")

        current_page_number_element = page.query_selector(".rdpCurrentPage")
        if current_page_number_element is None:
            return
        last_current_page_number = current_page_number_element.inner_text()
        print(f"finish running page number: {last_current_page_number}, moving to next page!")

        page.query_selector(".rdpPageNext").click()
        for _ in range(10):
            sleep(1)

            current_page_number_element = page.query_selector(".rdpCurrentPage")
            if current_page_number_element is None:
                return
            current_page_number = current_page_number_element.inner_text()

            if current_page_number != last_current_page_number:
                last_current_page_number = "-1"
                break
        if last_current_page_number != "-1":
            break
    print(f"found {len(names)} results :(")
    with open("btl.txt", "w+") as file:
        file.writelines(names)
