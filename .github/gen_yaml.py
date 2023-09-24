from jinja2 import Environment, FileSystemLoader

environment = Environment(loader=FileSystemLoader("./"))
template = environment.get_template("main.yml.j2")

with open("workflows/main.yml", mode="w") as f:
    f.write(template.render(
        {
            "buildconfigs": [
                { "name": "pico",  "cmake_opts": ""},
            ]
        }
    ))

