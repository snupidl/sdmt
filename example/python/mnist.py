"""
Copyright 2021 PIDL(Petabyte-scale In-memory Database Lab) http://kdb.snu.ac.kr
This work was supported by Next-Generation Information Computing Development
Program through the National Research Foundation of Korea(NRF)
funded by the Ministry of Science, ICT (NRF-2016M3C4A7952587)
Author: Ilju Lee, Jongin Kim, Hyerim Jeon, Youngjune Park
Contact: sdmt@kdb.snu.ac.kr
"""

# import sdmt
from sdmt import sdmt

# import mpi module
from mpi4py import MPI

# tensorflow, numpy
from tensorflow.examples.tutorials.mnist import input_data
import tensorflow as tf
import numpy as np

# load data
mnist = input_data.read_data_sets("MNIST_data/", one_hot=True)

# init sdmt module
sdmt.init('./config_python_test.xml', True)

# check this execution is first try
# if it is, create segments
# else get recovered value
if not sdmt.exist('mnist_W'):
    sdmt.register_snapshot('mnist_W', sdmt.vt.float, sdmt.dt.matrix, [784, 10])
    sdmt.register_snapshot('mnist_b', sdmt.vt.float, sdmt.dt.array, [10])

    segment_W = np.array(sdmt.get('mnist_W'), copy=False)
    segment_b = np.array(sdmt.get('mnist_b'), copy=False)

    for i in range(784):
        for j in range(10):
            segment_W[i, j] = 0
    for i in range(10):
        segment_b[i] = 0
else:
    segment_W = np.array(sdmt.get('mnist_W'), copy=False)
    segment_b = np.array(sdmt.get('mnist_b'), copy=False)

# set variables
x = tf.placeholder(tf.float32, [None, 784])
W = tf.Variable(segment_W)
b = tf.Variable(segment_b)
y = tf.nn.softmax(tf.matmul(x, W) + b)

# define loss function(cross-entropy)
y_ = tf.placeholder(tf.float32, [None, 10])
loss = tf.reduce_mean(-tf.reduce_sum(y_ * tf.log(y), reduction_indices=[1]))
train_step = tf.train.GradientDescentOptimizer(0.5).minimize(loss)

# function that calculates accuracy
correct_prediction = tf.equal(tf.argmax(y,1), tf.argmax(y_,1))
accuracy = tf.reduce_mean(tf.cast(correct_prediction, tf.float32))

# get current iteration sequence
it = sdmt.iter()
print('current iteration order: ', it)

# start sdmt module
sdmt.start();

# open session
with tf.Session() as sess:
    sess.run(tf.global_variables_initializer())

    while it < 10000:
        batch_xs, batch_ys = mnist.train.next_batch(100)

        # check accuracy and loss for every 100 steps
        # and checkpoint snapshot of trained model
        if it % 100 == 0:
            # copy intermediate to sdmt segment
            # then checkpoint snapshot
            w_tmp = sess.run(W)
            b_tmp = sess.run(b)
            for i in range(784):
                for j in range(10):
                    segment_W[i, j] = w_tmp[i, j]
            for i in range(10):
                segment_b[i] = b_tmp[i]

            sdmt.checkpoint(1)

            # calculate accuacy and loss
            train_loss = sess.run(
                    loss, feed_dict={x: batch_xs, y_: batch_ys})
            train_accuracy = sess.run(
                    accuracy, feed_dict={x: batch_xs, y_: batch_ys})

            print("Epoch: %d, train accuracy: %f, loss: %f"
                    % (it, train_accuracy, train_loss))

        sess.run(train_step, feed_dict={x: batch_xs, y_: batch_ys})

        # move to next iteration
        it = sdmt.next()

    print(sess.run(accuracy, feed_dict={x: mnist.test.images, y_: mnist.test.labels}))

# finalize sdmt module
sdmt.finalize();
