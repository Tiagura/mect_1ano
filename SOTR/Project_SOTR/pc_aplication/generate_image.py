from PIL import Image, ImageDraw
import random
import os 

# DEFINE
IMAGE_WIDTH             = 128 # size of image
BACKGROUND_COLOR        = 0   # black
LINE_COLOR              = 255 # white
OBSTACLE_COLOR          = 128 # gray
MAX_NUMBER_OF_OBSTACLES = 50  # max number of obstacles
images_generated        = 0   # number of images to generate

def get_white_pixeis(img:Image): # get all white pixels
    white_pixeis = []
    for x in range(img.size[0]):
        for y in range(img.size[1]):
            if img.getpixel((x, y)) == LINE_COLOR:
                white_pixeis.append((x, y))
    return white_pixeis

# Cria uma imagem em preto com o tamanho especificado em pixels - IMAGE_WIDTHxIMAGE_WIDTH
def generate_image(name:str):
    
    img = Image.new('L', (IMAGE_WIDTH, IMAGE_WIDTH), color = BACKGROUND_COLOR)  # mono = 'L'

    # Obtém um objeto para desenhar na imagem
    draw = ImageDraw.Draw(img)

    # Desenha uma linha branca de 1 pixel de espessura do topo até ao fundo da imagem
    value_top = random.randint(0, IMAGE_WIDTH - 1)                                          # random value between 0 and IMAGE_WIDTH - 1
    value_bottom = random.randint(0, IMAGE_WIDTH - 1)                                       # random value between 0 and IMAGE_WIDTH - 1
    draw.line((value_top, 0, value_bottom, IMAGE_WIDTH - 1), fill=LINE_COLOR, width=1)      # draw line from top to bottom

    obstacles = random.randint(1, MAX_NUMBER_OF_OBSTACLES)                                  # random number of obstacles between 1 and MAX_NUMBER_OF_OBSTACLES

    white_pixeis = get_white_pixeis(img)                                                    # get all white pixels
    
    for i in range(obstacles):                                                              # draw obstacles
        x = random.randint(0, IMAGE_WIDTH - 1)                                              # random x, y value between 0 and IMAGE_WIDTH - 1
        y = random.randint(0, IMAGE_WIDTH - 1)                                              
        cond = (x, y) in white_pixeis                                                       # check if the pixel is white

        while (cond):                                                                       # if the pixel is white, generate another one
            x = random.randint(0, IMAGE_WIDTH - 1)                                          # random x, y value between 0 and IMAGE_WIDTH - 1
            y = random.randint(0, IMAGE_WIDTH - 1)                                          
            cond = (x, y) in white_pixeis                                                   # check if the pixel is white

        draw.point((x, y), fill=OBSTACLE_COLOR)                                             # draw obstacle
        if (x+1, y) in white_pixeis:                                                        # if the pixel next to the obstacle is white, draw obstacle with :
            draw.point((x-1, y), fill=OBSTACLE_COLOR)                                       # left side
        else:
            draw.point((x+1, y), fill=OBSTACLE_COLOR)                                       # right side

    
    
    
    save_hex_image(img, name) # save image in raw format
    img.save(f'imagens/{name}.png')  # save as png

def save_hex_image(img:Image, name:str):
    with open(f'imagens/{name}.raw', 'wb') as f:
        for x in range(img.size[0]):
            for y in range(img.size[1]):
                f.write(bytes([img.getpixel((x, y))]))

if __name__ == '__main__':
    index_image = 0

    images_generated = int(input('Number of images to generate: ')) # number of images to generate
    
    for file in os.listdir('imagens'): # remove all content from path imagens
        os.remove(os.path.join('imagens', file))

    print('All images removed from imagens folder !\n') 

    for i in range(images_generated): # generate number of 'images_generated' images
        generate_image(f'img{index_image}')                             # generate image                              
        print('Image {:03d} generated'.format(index_image))             # status of image generation
        index_image += 1                                                # increment index_image
