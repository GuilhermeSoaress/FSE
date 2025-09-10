from PIL import Image, ImageDraw, ImageFont
import Adafruit_SSD1306

# Define o pino de reset (RST). Defina como None se não estiver usando.
RST = None

# Cria uma instância do display OLED com I2C
disp = Adafruit_SSD1306.SSD1306_128_64(rst=RST)

# Inicializa o display
disp.begin()

# Limpa o display
disp.clear()
disp.display()

# Cria um objeto de imagem em preto e branco
width = disp.width
height = disp.height
image = Image.new('1', (width, height))

# Objeto de desenho para desenhar na imagem
draw = ImageDraw.Draw(image)

# Define uma fonte padrão
font = ImageFont.load_default()

def clear_display():
    disp.clear()
    disp.display()

def display_text(line1, line2):
    # Limpa a imagem com um retângulo preto
    draw.rectangle((0, 0, width, height), outline=0, fill=0)
    
    # Escreve o texto nas linhas especificadas
    draw.text((0, 0), line1, font=font, fill=255)
    draw.text((0, 20), line2, font=font, fill=255)
    
    # Exibe a imagem no display
    disp.image(image)
    disp.display()

clear_display()
