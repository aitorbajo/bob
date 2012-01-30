#!/usr/bin/env python

"""Tests on the LinearScoring function
"""

import os, sys
import unittest
import bob
import numpy

class LinearScoringTest(unittest.TestCase):
  """Performs various LinearScoring tests."""

  def test01_LinearScoring(self):
    ubm = bob.machine.GMMMachine(2, 2)
    ubm.weights   = numpy.array([0.5, 0.5], 'float64')
    ubm.means     = numpy.array([[3, 70], [4, 72]], 'float64')
    ubm.variances = numpy.array([[1, 10], [2, 5]], 'float64')
    ubm.varianceThresholds = numpy.array([[0, 0], [0, 0]], 'float64')

    model1 = bob.machine.GMMMachine(2, 2)
    model1.weights   = numpy.array([0.5, 0.5], 'float64')
    model1.means     = numpy.array([[1, 2], [3, 4]], 'float64')
    model1.variances = numpy.array([[9, 10], [11, 12]], 'float64')
    model1.varianceThresholds = numpy.array([[0, 0], [0, 0]], 'float64')
    
    model2 = bob.machine.GMMMachine(2, 2)
    model2.weights   = numpy.array([0.5, 0.5], 'float64')
    model2.means     = numpy.array([[5, 6], [7, 8]], 'float64')
    model2.variances = numpy.array([[13, 14], [15, 16]], 'float64')
    model2.varianceThresholds = numpy.array([[0, 0], [0, 0]], 'float64')
    
    stats1 = bob.machine.GMMStats(2, 2)
    stats1.sumPx = numpy.array([[1, 2], [3, 4]], 'float64')
    stats1.n = numpy.array([1, 2], 'float64')
    stats1.T = 1+2
    
    stats2 = bob.machine.GMMStats(2, 2)
    stats2.sumPx = numpy.array([[5, 6], [7, 8]], 'float64')
    stats2.n = numpy.array([3, 4], 'float64')
    stats2.T = 3+4
    
    stats3 = bob.machine.GMMStats(2, 2)
    stats3.sumPx = numpy.array([[5, 6], [7, 3]], 'float64')
    stats3.n = numpy.array([3, 4], 'float64')
    stats3.T = 3+4

    test_channeloffset = [numpy.array([9, 8, 7, 6], 'float64'), numpy.array([5, 4, 3, 2], 'float64'), numpy.array([1, 0, 1, 2], 'float64')]

    # Reference scores (from Idiap internal matlab implementation)
    ref_scores_00 = numpy.array([[2372.9, 5207.7, 5275.7], [2215.7, 4868.1, 4932.1]], 'float64')
    ref_scores_01 = numpy.array( [[790.9666666666667, 743.9571428571428, 753.6714285714285], [738.5666666666667, 695.4428571428572, 704.5857142857144]], 'float64')
    ref_scores_10 = numpy.array([[2615.5, 5434.1, 5392.5], [2381.5, 4999.3, 5022.5]], 'float64')
    ref_scores_11 = numpy.array([[871.8333333333332, 776.3000000000001, 770.3571428571427], [793.8333333333333, 714.1857142857143, 717.5000000000000]], 'float64')


    # 1/ Use GMMMachines
    # 1/a/ Without test_channelOffset, without frame-length normalisation
    scores = bob.machine.linearScoring([model1, model2], ubm, [stats1, stats2, stats3])
    self.assertTrue((abs(scores - ref_scores_00) < 1e-7).all())
    
    # 1/b/ Without test_channelOffset, with frame-length normalisation
    scores = bob.machine.linearScoring([model1, model2], ubm, [stats1, stats2, stats3], [], True)
    self.assertTrue((abs(scores - ref_scores_01) < 1e-7).all())

    # 1/c/ With test_channelOffset, without frame-length normalisation
    scores = bob.machine.linearScoring([model1, model2], ubm, [stats1, stats2, stats3], test_channeloffset)
    self.assertTrue((abs(scores - ref_scores_10) < 1e-7).all())

    # 1/d/ With test_channelOffset, with frame-length normalisation
    scores = bob.machine.linearScoring([model1, model2], ubm, [stats1, stats2, stats3], test_channeloffset, True)
    self.assertTrue((abs(scores - ref_scores_11) < 1e-7).all())

    
    # 2/ Use mean/variance supervectors
    # 2/a/ Without test_channelOffset, without frame-length normalisation
    scores = bob.machine.linearScoring([model1.meanSupervector, model2.meanSupervector], ubm.meanSupervector, ubm.varianceSupervector, [stats1, stats2, stats3])
    self.assertTrue((abs(scores - ref_scores_00) < 1e-7).all())

    # 2/b/ Without test_channelOffset, with frame-length normalisation
    scores = bob.machine.linearScoring([model1.meanSupervector, model2.meanSupervector], ubm.meanSupervector, ubm.varianceSupervector, [stats1, stats2, stats3], [], True)
    self.assertTrue((abs(scores - ref_scores_01) < 1e-7).all())

    # 2/c/ With test_channelOffset, without frame-length normalisation
    scores = bob.machine.linearScoring([model1.meanSupervector, model2.meanSupervector], ubm.meanSupervector, ubm.varianceSupervector, [stats1, stats2, stats3], test_channeloffset)
    self.assertTrue((abs(scores - ref_scores_10) < 1e-7).all())

    # 2/d/ With test_channelOffset, with frame-length normalisation
    scores = bob.machine.linearScoring([model1.meanSupervector, model2.meanSupervector], ubm.meanSupervector, ubm.varianceSupervector, [stats1, stats2, stats3], test_channeloffset, True)
    self.assertTrue((abs(scores - ref_scores_11) < 1e-7).all())

   
if __name__ == '__main__':
  sys.argv.append('-v')
  if os.environ.has_key('BOB_PROFILE') and \
      os.environ['BOB_PROFILE'] and \
      hasattr(bob.core, 'ProfilerStart'):
    bob.core.ProfilerStart(os.environ['BOB_PROFILE'])
  os.chdir(os.path.realpath(os.path.dirname(sys.argv[0])))
  unittest.main()
  if os.environ.has_key('BOB_PROFILE') and \
      os.environ['BOB_PROFILE'] and \
      hasattr(bob.core, 'ProfilerStop'):
    bob.core.ProfilerStop()
