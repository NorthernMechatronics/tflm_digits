import numpy as np
import keras

def create_model(image_size):
    keras.backend.clear_session()
    # Reference model
    # model = keras.Sequential([
    #     keras.layers.Conv2D(32, (3, 3), padding='same',
    #                         activation='relu',
    #                         input_shape=image_size),
    #     keras.layers.BatchNormalization(),

    #     keras.layers.Conv2D(32, (3, 3), padding='same',
    #                         activation='relu'),
    #     keras.layers.MaxPooling2D((2, 2)),
    #     keras.layers.Dropout(0.4),

    #     keras.layers.Conv2D(64, (3, 3), padding='same',
    #                         activation='relu'),
    #     keras.layers.BatchNormalization(),

    #     keras.layers.Conv2D(64, (3, 3), padding='same',
    #                         activation='relu'),
    #     keras.layers.MaxPooling2D((2, 2)),
    #     keras.layers.Dropout(0.4),

    #     keras.layers.Conv2D(128, (2, 2), padding='same',
    #                         activation='relu'),
    #     keras.layers.BatchNormalization(),
    #     keras.layers.MaxPooling2D((2, 2)),
    #     keras.layers.Dropout(0.4),

    #     keras.layers.Flatten(),
    #     keras.layers.Dense(128, activation='relu'),
    #     keras.layers.Dropout(0.4),
    #     keras.layers.Dense(10, activation='softmax')
    # ])

    # Optimised model
    model = keras.Sequential([
        keras.layers.Conv2D(32, kernel_size=(3, 3), padding='same',
                            activation='relu',
                            input_shape=image_size),

        keras.layers.Conv2D(32, kernel_size=(3, 3), padding='same',
                            activation='relu'),
        keras.layers.MaxPooling2D((2, 2)),
        keras.layers.Dropout(0.25),

        keras.layers.Conv2D(32, kernel_size=(3, 3), padding='same',
                            activation='relu'),
        keras.layers.BatchNormalization(),
        keras.layers.MaxPooling2D((2, 2)),
        keras.layers.Dropout(0.25),

        keras.layers.Conv2D(32, kernel_size=(3, 3), padding='same',
                            activation='relu'),
        keras.layers.MaxPooling2D((2, 2)),
        keras.layers.Dropout(0.25),

        keras.layers.Conv2D(32, kernel_size=(3, 3), padding='same',
                            activation='relu'),
        keras.layers.BatchNormalization(),
        keras.layers.MaxPooling2D((2, 2)),
        keras.layers.Dropout(0.25),

        keras.layers.Flatten(),
        keras.layers.Dense(128, activation='relu'),
        keras.layers.Dropout(0.5),
        keras.layers.Dense(10, activation='softmax')
    ])


    optimizer = keras.optimizers.Adam(learning_rate=1e-3, amsgrad=True)
    model.compile(optimizer=optimizer,
                loss='categorical_crossentropy',
                metrics=['accuracy'])

    model.summary()
    return model

